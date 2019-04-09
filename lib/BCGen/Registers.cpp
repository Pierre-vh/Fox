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
  // Use the hint if possible
  if (hint) {
    assert(hint->canRecycle() && "Hint is not recyclable");
    // Recycle the hint as the new address
    data.addr = rawRecycleRegister(std::move(*hint));
  } 
  // Else just use a new register
  else 
    data.addr = rawAllocateNewRegister();
  // If we're in a loop, notify the LoopContext that this variable
  // was declared inside it.
  if (isInLoop()) {
    curLoopContext_->varsInLoop_.insert(var);
  }
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
  // First, check that every variable declared inside the loop context
  // was indeed freed. 
  assert((lc.varsInLoop_.size() == 0)
    && "Some variables declared inside the loop were still alive "
       "at the destruction of the LoopContext");
  // Now free every variable in the "delayedFrees" set
  for (auto var : lc.delayedFrees_) {
    // Search for the var and fetch its data
    auto it = knownVars_.find(var);
    assert((it != knownVars_.end()) && "Unknown Variable!");
    VarData& data = it->second;
    // Check that the variable is dead
    assert((data.useCount == 0)
      && "a variable part of LoopContext::delayedFrees_ was not "
         "dead");
    // Free it
    markRegisterAsFreed(data.addr.getValue());
    // Remove the entry from the map
    knownVars_.erase(it);
  }
}

regaddr_t RegisterAllocator::rawRecycleRegister(RegisterValue value) {
  assert(value.canRecycle() && "register not recyclable");
  regaddr_t addr = value.getAddress();
  switch (value.getKind()) {
    case RegisterValue::Kind::Temporary:
      // Nothing to do for temporaries except killing it
      value.kill();
      return addr;
    case RegisterValue::Kind::Var: {
      // Search for the var
      auto it = knownVars_.find(value.data_.varDecl);
      assert((it != knownVars_.end()) && "Unknown Variable!");
      // Forget it, kill 'value' and return
      knownVars_.erase(it);
      value.kill();
      return addr;
    }
    default:
      fox_unreachable("Unknown RegisterValue::Kind");
  }
}

regaddr_t RegisterAllocator::rawAllocateNewRegister() {
  // Try to compact the freeRegisters_ set
  // FIXME: Is this a good idea to call this every alloc? 
  //        The method is fairly cheap so it shouldn't be an issue, 
  //        but some profiling wouldn't hurt!
  compactFreeRegisterSet();

  // If we have something in freeRegisters_, use that.
  if (!freeRegisters_.empty()) {
    // Take the smallest number possible (to reuse registers whose number
    // is as small as possible, so compactFreeRegisterSet() is more
    // efficient)
    auto pick = --freeRegisters_.end();
    regaddr_t reg = (*pick);
    freeRegisters_.erase(pick);
    return reg;
  }

  // Check that we haven't allocated too many registers.
  assert((biggestAllocatedReg_ != max_regaddr) && 
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
    assert((biggestAllocatedReg_ > reg) 
      && "Register maybe freed twice");
    // Only capture the result of std::set::insert in debug builds to avoid
    // "unused variable" errors in release builds (where asserts are disabled)
    #ifndef NDEBUG
      auto insertResult =
    #endif

    freeRegisters_.insert(reg);

    assert(insertResult.second && "Register maybe freed twice: "
    " It was already in freeRegisters_");
  }
}

regaddr_t RegisterAllocator::getRegisterOfVar(const VarDecl* var) const {
  // Search for the var
  auto it = knownVars_.find(var);
  assert((it != knownVars_.end()) && "Unknown Variable!");
  // If this function is called, we should have a register reserved for this
  // variable
  assert(it->second.hasAddress() && "Variable doesn't have an address!");
  return it->second.addr.getValue();
}

void RegisterAllocator::release(const VarDecl* var) {
  // Search for the var and fetch its data
  auto it = knownVars_.find(var);
  assert((it != knownVars_.end()) && "Unknown Variable!");
  VarData& data = it->second;
  // Decrement the use count, asserting that it isn't 0 already
  assert((data.useCount != 0) && "Variable is already dead");
  --(data.useCount);
  // Check if the variable is dead after decrementation
  if (data.useCount == 0) {
    // If we are inside a loop, we must check that the variable
    // is part of this LoopContext.
    // If that's the case, we can free it, else, we must
    // delay its death until the end of the LC.
    if (isInLoop()) {
      if (!curLoopContext_->isVarDeclaredInside(var)) {
        curLoopContext_->delayedFrees_.insert(var);
        return;
      }
    }
    // Free the register.
    assert(data.hasAddress() && "Variable doesn't have an address!");
    markRegisterAsFreed(data.addr.getValue());
    // Remove the entry from the map
    knownVars_.erase(it);
  }
}

bool RegisterAllocator::isLastUsage(const VarDecl* var) const {
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
      return getRegisterAllocator()->isLastUsage(data_.varDecl);
    default:
      fox_unreachable("unknown RegisterValue::Kind");
  }
}

RegisterValue::operator bool() const {
  return isAlive();
}

void RegisterValue::free() {
  // We can't free dead RVs.
  if(!isAlive()) return;
  // Do what we have to do.
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
  // Kill this object to doing it twice.
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