//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Registers.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Registers.hpp"
#include "Fox/Common/Errors.hpp"
#include "LoopContext.hpp"
#include <utility>

using namespace fox;

//----------------------------------------------------------------------------//
// RegisterAllocator
//----------------------------------------------------------------------------//

void RegisterAllocator::addUsage(const VarDecl* var) {
  ++(knownVars_[var].useCount);
}

RegisterValue 
RegisterAllocator::initVar(const VarDecl* var, RegisterValue* hint) {
  // Search for the var
  auto it = knownVars_.find(var);
  assert((it != knownVars_.end()) && "Unknown Variable!");

  // Assert that the variable has not been initialized yet
  VarData& data = it->second;
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
    curLoopContext_->varsInLoop_.insert(var);

  // Return a RegisterValue managing this Var
  return RegisterValue(this, var);
}

RegisterValue RegisterAllocator::useVar(const VarDecl* var) {
  // Search for the var
  auto it = knownVars_.find(var);
  assert((it != knownVars_.end()) && "Unknown Variable!");

  // Assert that the variable has been assigned a register.
  assert(it->second.hasAddress() && "Var has not been initialized "
    "(initVar not called for this variable)");

  // Return a RegisterValue managing this Var
  return RegisterValue(this, var);
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
  // Assert that all variables declared inside this LC have been
  // released.
  assert((lc.varsInLoop_.size() == 0)
    && "Some variables declared inside the loop were still alive "
       "at the destruction of the LoopContext");
  // Release every variable in the "delayedFrees" set
  for (auto var : lc.delayedReleases_) 
    release(var, /*alreadyDead*/ true);
}

regaddr_t RegisterAllocator::rawRecycleRegister(RegisterValue value) {
  assert(value.canRecycle() && "register not recyclable");
  regaddr_t addr = value.getAddress();
  switch (value.getKind()) {
    // Nothing to do for temporaries
    case RegisterValue::Kind::Temporary: break;
    case RegisterValue::Kind::Var:
      forgetVariable(knownVars_.find(value.data_.varDecl));
      break;
    default:
      fox_unreachable("Unknown RegisterValue::Kind");
  }
  value.kill();
  return addr;
}

void RegisterAllocator::forgetVariable(KnownVarsMap::iterator iter) {
  assert((iter != knownVars_.end()) && "unknown var");

  // Remove it from the current LoopContext if needed
  if (isInLoop()) {
    auto& lc_knownVars = curLoopContext_->varsInLoop_;
    auto lc_iter = lc_knownVars.find(iter->first);
    if(lc_iter != lc_knownVars.end())
      lc_knownVars.erase(lc_iter);
  }

  // Remove it from the set of known variables
  knownVars_.erase(iter);
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

regaddr_t RegisterAllocator::getRegisterOfVar(const VarDecl* var) const {
  // Search for the var
  auto it = knownVars_.find(var);
  assert((it != knownVars_.end()) && "Unknown Variable!");
  // If this function is called, we should have a register reserved for this 
  // variable
  assert(it->second.hasAddress() && "Variable doesn't have an address!");
  assert(it->second.useCount && "Variable is dead");
  return it->second.addr.getValue();
}

void RegisterAllocator::release(const VarDecl* var, bool isAlreadyDead) {
  // Search for the var and fetch its data
  auto it = knownVars_.find(var);
  assert((it != knownVars_.end()) && "Unknown Variable!");
  VarData& data = it->second;

  // Decrement the use count (if needed)
  if(isAlreadyDead) assert(data.useCount == 0);
  else {
    assert((data.useCount != 0) && "Variable is already dead");
    --(data.useCount);
  }

  // Check if the variable is dead
  if (data.useCount == 0) {
    // In loops, we can't free variables declared outside the loop
    if (isInLoop()) {
      if (!curLoopContext_->isVarDeclaredInside(var)) {
        // Delay the release until the end of the LC's lifetime.
        curLoopContext_->delayedReleases_.insert(var);
        return;
      }
    }
    // Free its register
    markRegisterAsFreed(data.addr.getValue());
    // Forget the variable
    forgetVariable(it);
  }
}

bool RegisterAllocator::canRecycle(const VarDecl* var) const {
  if (isInLoop()) {
    // Can only recycle vars declared inside this LC
    if(!curLoopContext_->isVarDeclaredInside(var))
      return false;
  }
  auto it = knownVars_.find(var);
  assert((it != knownVars_.end()) && "Unknown Variable!");
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
    case Kind::Var:
      // Normally, having to lookup in the map each time shouldn't be a
      // big deal since the result of getAddress is usually saved
      // and lookup in hashmaps are pretty cheap. If it turns out to
      // be a performance issue, cache the address in the RegisterValue
      // directly, even if it increases its size.
      return getRegisterAllocator()->getRegisterOfVar(data_.varDecl);
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

bool RegisterValue::isVar() const {
  return (getKind() == Kind::Var);
}

bool RegisterValue::canRecycle() const {
  if(!isAlive()) return false;
  switch (getKind()) {
    case Kind::Temporary:
      return true;
    case Kind::Var:
      return getRegisterAllocator()->canRecycle(data_.varDecl);
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
    case Kind::Var:
      getRegisterAllocator()->release(data_.varDecl);
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

RegisterValue::RegisterValue(RegisterAllocator* regAlloc, const VarDecl* var) :
  regAllocAndKind_(regAlloc, Kind::Var) {
  data_.varDecl = var;
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
