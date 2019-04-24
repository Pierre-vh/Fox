//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Registers.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Registers.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/Common/Errors.hpp"
#include "LoopContext.hpp"
#include <utility>

using namespace fox;

//----------------------------------------------------------------------------//
// RegisterAllocator
//----------------------------------------------------------------------------//

RegisterAllocator::RegisterAllocator(ParamList* params) {
  if(!params) return; // Nothing to do
  assert((biggestAllocatedReg_ == 0) 
    && "incorrect starting value for biggestAllocatedReg_");
  for (ParamDecl* param : *params) {
    assert((biggestAllocatedReg_ != bc_limits::max_regaddr) && 
      "Can't allocate more registers : Register number limit reached "
      "(too much register pressure) because a function has too many params");
    DeclData& data = knownDecls_[param];
    // This doesn't count as an usage of the ParamDecl, so set useCount to 0
    data.useCount = 0;
    // Assign a register address equal to the index of the param
    data.addr = biggestAllocatedReg_++;
    // We cannot free the registers used by mutable parameters
    data.canFree = !param->isMut();
  }
}

void RegisterAllocator::addUsage(const ValueDecl* decl) {
  ++(knownDecls_[decl].useCount);
}

void RegisterAllocator::
freeUnusedParameters(ParamList* params, 
                     SmallVectorImpl<const ParamDecl*>& unused) {
  if(!params) return;
  for (ParamDecl* param : *params) {
    auto it = knownDecls_.find(param);
    assert((it != knownDecls_.end()) && "unknown param");
    DeclData& dd = it->second;
    if(dd.useCount != 0) continue;
    // Variable is unused, free it.
    release(param, /*alreadyDead*/ true, /*freeProtected*/ true);
    // Add it to the unused vector
    unused.push_back(param);
  }
}

RegisterValue 
RegisterAllocator::initVar(const VarDecl* var, RegisterValue* hint) {
  // Search for the var
  auto it = knownDecls_.find(var);
  assert((it != knownDecls_.end()) && "Unknown Variable!");

  // Assert that the variable has not been initialized yet
  DeclData& data = it->second;
  assert(!data.hasAddress() && "Var has already been initialized:"
    "(initVar already called for this variable)");

  // Use the hint if possible, else just use a new register.
  if (hint) {
    assert(hint->canRecycle() && "Hint is not recyclable");
    // Recycle the hint as the new address
    data.addr = rawRecycleRegister(std::move(*hint));
  } 
  else 
    data.addr = rawAllocateNewRegister();

  // If we're in a loop, notify the LoopContext that this variable
  // was declared inside it.
  if (isInLoop())
    curLoopContext_->declsInLoop_.insert(var);

  // Return a RegisterValue managing this Var
  return RegisterValue(this, var);
}

RegisterValue RegisterAllocator::useDecl(const ValueDecl* decl) {
#ifndef NDEBUG
  // Search for the var
  auto it = knownDecls_.find(decl);
  assert((it != knownDecls_.end()) && "Unknown Decl!");

  // Assert that the variable has been assigned a register.
  assert(it->second.hasAddress() && "Decl has not been initialized "
    "(initVar not called for this variable)");
#endif
  // Return a RegisterValue managing a use of this Decl.
  return RegisterValue(this, decl);
}

RegisterValue RegisterAllocator::allocateTemporary() {
  return RegisterValue(this, rawAllocateNewRegister());
}

RegisterValue RegisterAllocator::recycle(RegisterValue value) {
  return RegisterValue(this, rawRecycleRegister(std::move(value)));
}

regaddr_t RegisterAllocator::numbersOfRegisterInUse() const {
  regaddr_t num = biggestAllocatedReg_;
  for (auto elem : freeRegisters_)
    if(elem < biggestAllocatedReg_) --num;
  return num;
}

bool RegisterAllocator::isInLoop() const {
  return curLoopContext_;
}

void RegisterAllocator::actOnEndOfLoopContext(LoopContext& lc) {
  // Assert that all decls declared inside this LC have been
  // released.
  assert((lc.declsInLoop_.size() == 0)
    && "Some Decls declared inside the loop were still alive "
       "at the destruction of the LoopContext");
  // Release every decl in the delayedReleases set
  for (auto decl : lc.delayedReleases_) 
    release(decl, /*alreadyDead*/ true);
}

regaddr_t RegisterAllocator::rawRecycleRegister(RegisterValue value) {
  assert(value.canRecycle() && "register not recyclable");
  regaddr_t addr = value.getAddress();
  switch (value.getKind()) {
    // Nothing to do for temporaries
    case RegisterValue::Kind::Temporary: break;
    case RegisterValue::Kind::DeclRef:
      forgetDecl(knownDecls_.find(value.data_.decl));
      break;
    default:
      fox_unreachable("Unknown RegisterValue::Kind");
  }
  value.kill();
  return addr;
}

void RegisterAllocator::forgetDecl(KnownDeclsMap::iterator iter) {
  assert((iter != knownDecls_.end()) && "unknown decl");

  // Remove it from the current LoopContext if needed
  if (isInLoop()) {
    auto& knownDecls_ = curLoopContext_->declsInLoop_;
    auto lc_iter = knownDecls_.find(iter->first);
    if(lc_iter != knownDecls_.end())
      knownDecls_.erase(lc_iter);
  }

  // Remove it from the set of known declarations
  knownDecls_.erase(iter);
}

regaddr_t RegisterAllocator::rawAllocateNewRegister() {
  compactFreeRegisterSet();

  // If we have something in freeRegisters_, use that.
  if (!freeRegisters_.empty()) {
    // Take the smallest number possible
    auto pick = --freeRegisters_.end();
    regaddr_t reg = (*pick);
    freeRegisters_.erase(pick);
    return reg;
  }

  // Else we'll need to allocate a new register, so check
  // if that's possible.
  // TODO: Replace this with a proper diagnostic
  assert((biggestAllocatedReg_ != bc_limits::max_regaddr) && 
    "Can't allocate more registers : Register number limit reached "
    "(too much register pressure)");

  // Return biggestAllocatedReg_ then increment it.
  return biggestAllocatedReg_++;
 
}

void RegisterAllocator::markRegisterAsFreed(regaddr_t reg) {
  // Check if we can mark the register as freed by merely decrementing
  // biggestAllocatedReg_
  if((reg+1) == biggestAllocatedReg_)
    biggestAllocatedReg_--;
  // Else, add it to the free registers set
  else {
    assert((biggestAllocatedReg_ > reg) && "Register free twice");

    #ifndef NDEBUG
      auto insertResult =
    #endif
    freeRegisters_.insert(reg);

    assert(insertResult.second && "Register freed twice, it was already "
      "in freeRegisters");
  }
}

regaddr_t RegisterAllocator::getRegisterOfDecl(const ValueDecl* decl) const {
  // Search for the decl
  auto it = knownDecls_.find(decl);
  assert((it != knownDecls_.end()) && "Unknown Decl!");
  // If this function is called, we should have a register reserved for this 
  // decl
  assert(it->second.hasAddress() && "Decl doesn't have an address!");
  assert(it->second.useCount && "Decl is dead");
  return it->second.addr.getValue();
}

void RegisterAllocator::release(const ValueDecl* decl, 
                                bool isAlreadyDead, 
                                bool freeProtected) {
  // Search for the Decl and fetch its data
  auto it = knownDecls_.find(decl);
  assert((it != knownDecls_.end()) && "Unknown Decl!");
  DeclData& data = it->second;

  // Decrement the use count (if needed)
  if(isAlreadyDead) assert(data.useCount == 0);
  else {
    assert((data.useCount != 0) && "Decl is already dead");
    --(data.useCount);
  }

  // Check if the Decl is dead
  if (data.useCount == 0) {
    // Don't free it if we're not allowed to.
    if(!freeProtected && !data.canFree) return;
    // In loops, we can't free registers used by Decls declared 
    // outside the loop
    if (isInLoop()) {
      if (!curLoopContext_->isDeclaredInside(decl)) {
        // Delay the release until the end of the LC's lifetime.
        curLoopContext_->delayedReleases_.insert(decl);
        return;
      }
    }
    // Check if we are freeing a ParamDecl
    // Free its register
    markRegisterAsFreed(data.addr.getValue());
    // Forget the Decl
    forgetDecl(it);
  }
}

bool RegisterAllocator::canRecycle(const ValueDecl* decl) const {
  if (isInLoop()) {
    // Can only recycle decls declared inside this LC
    if(!curLoopContext_->isDeclaredInside(decl))
      return false;
  }
  auto it = knownDecls_.find(decl);
  assert((it != knownDecls_.end()) && "Unknown Decl!");
  return (it->second.useCount == 1);
}

void RegisterAllocator::compactFreeRegisterSet() {
  // Compacting is not needed if we haven't allocated any regs yet,
  // or if freeRegisters_ is empty.
  if(biggestAllocatedReg_ == 0) return;
  if(freeRegisters_.empty()) return;

  while (true) {
    // If the highest entry in freeRegisters_ is equivalent to
    // biggestAllocatedReg_-1, remove it and decrement 
    // biggestAllocatedReg_. Else, return.
    auto it = freeRegisters_.begin();
    if(it == freeRegisters_.end()) return;
    if((*it) != (biggestAllocatedReg_-1)) return;
    freeRegisters_.erase(it); // erase the element
    --biggestAllocatedReg_;   // decrement biggestAllocatedReg_
  }
}

//----------------------------------------------------------------------------//
// RegisterValue
//----------------------------------------------------------------------------//


RegisterValue::RegisterValue(RegisterValue&& other) {
  (*this) = std::move(other);
}

RegisterValue::~RegisterValue() {
  free();
}

RegisterValue::Kind RegisterValue::getKind() const {
  return regAllocAndKind_.getInt();
}

RegisterValue& RegisterValue::operator=(RegisterValue&& other) {
  free();
  regAllocAndKind_ = std::move(other.regAllocAndKind_);
  data_ = std::move(other.data_);
  other.kill();
  return *this;
}

regaddr_t RegisterValue::getAddress() const {
  assert(isAlive() 
    && "Cannot take the address of a dead RegisterValue");
  switch (getKind()) {
    case Kind::Temporary:
      return data_.tempRegAddress;
    case Kind::DeclRef:
      // Normally, having to lookup in the map each time shouldn't be a
      // big deal since the result of getAddress is usually saved
      // and lookup in hashmaps are pretty cheap. If it turns out to
      // be a performance issue, cache the address in the RegisterValue
      // directly, even if it increases its size.
      return getRegisterAllocator()->getRegisterOfDecl(data_.decl);
    default:
      fox_unreachable("unknown RegisterValue::Kind");
  }
}

bool RegisterValue::isAlive() const {
  // We're alive if our RegisterAllocator ptr is non null.
  return (bool)getRegisterAllocator();
}

bool RegisterValue::isTemporary() const {
  return (getKind() == Kind::Temporary);
}

bool RegisterValue::isDeclRef() const {
  return (getKind() == Kind::DeclRef);
}

bool RegisterValue::canRecycle() const {
  if(!isAlive()) return false;
  switch (getKind()) {
    case Kind::Temporary:
      return true;
    case Kind::DeclRef:
      return getRegisterAllocator()->canRecycle(data_.decl);
    default:
      fox_unreachable("unknown RegisterValue::Kind");
  }
}

RegisterValue::operator bool() const {
  return isAlive();
}

void RegisterValue::free() {
  if(!isAlive()) return;
  switch (getKind()) {
    case Kind::Temporary:
      getRegisterAllocator()->markRegisterAsFreed(data_.tempRegAddress);
      break;
    case Kind::DeclRef:
      getRegisterAllocator()->release(data_.decl);
      break;
    default:
      fox_unreachable("unknown RegisterValue::Kind");
  }
  kill();
}

RegisterAllocator* RegisterValue::getRegisterAllocator() {
  return regAllocAndKind_.getPointer();
}

const RegisterAllocator* RegisterValue::getRegisterAllocator() const {
  return regAllocAndKind_.getPointer();
}

RegisterValue::RegisterValue(RegisterAllocator* regAlloc, regaddr_t reg) :
  regAllocAndKind_(regAlloc, Kind::Temporary) {
  data_.tempRegAddress = reg;
}

RegisterValue::RegisterValue(RegisterAllocator* regAlloc, 
                             const ValueDecl* decl) :
  regAllocAndKind_(regAlloc, Kind::DeclRef) {
  data_.decl = decl;
}

void RegisterValue::kill() {
  regAllocAndKind_.setPointer(nullptr);
}

bool fox::operator==(const RegisterValue& lhs, const RegisterValue& rhs) {
  using Kind = RegisterValue::Kind;
  // Both are dead -> equal
  if(!lhs.isAlive() && !rhs.isAlive()) return true;
  // One is dead, the other isn't -> not equal
  if(!(lhs.isAlive() && rhs.isAlive()))
    return false;
  // Different kinds: not equal
  if(lhs.getKind() != rhs.getKind())
    return false;
  // Now just compare addresses
  return lhs.getAddress() == rhs.getAddress();
}

bool fox::operator!=(const RegisterValue& lhs, const RegisterValue& rhs) {
  return !(lhs == rhs);
}
