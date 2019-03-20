//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Registers.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Registers.hpp"
#include "Fox/Common/Errors.hpp"
#include <utility>

using namespace fox;

//----------------------------------------------------------------------------//
// RegisterAllocator
//----------------------------------------------------------------------------//

RegisterValue RegisterAllocator::allocateTemporary() {
  return RegisterValue(this, rawAllocateNewRegister());
}

void RegisterAllocator::addUsage(const VarDecl* var) {
  ++(knownVars_[var].useCount);
}

RegisterValue RegisterAllocator::getRegisterOfVar(const VarDecl* var) {
  // Search for the var
  auto it = knownVars_.find(var);
  assert((it != knownVars_.end()) && "Unknown Variable!");
  // Check if the variable has been assigned a register.
  // If it doesn't have a register, allocate one now.
  VarData& data = it->second;
  if(!data.hasAddress()) data.addr = rawAllocateNewRegister();
  // Return a RegisterValue managing this RegisterValue
  return RegisterValue(this, var);
}

regaddr_t RegisterAllocator::numbersOfRegisterInUse() const {
  regaddr_t num = biggestAllocatedReg_;
  for (auto elem : freeRegisters_)
    if(elem < biggestAllocatedReg_) --num;
  return num;
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
  // variable.
  assert(it->second.hasAddress() && "Variable doesn't have an address!");
  return it->second.addr.getValue();
}

void RegisterAllocator::release(const VarDecl* var) {
  // Search for the var
  auto it = knownVars_.find(var);
  assert((it != knownVars_.end()) && "Unknown Variable!");
  VarData& data = it->second;
  // Decrement the use count
  assert((data.useCount != 0) && "Variable is already dead");
  --(data.useCount);
  // Check if the variable is dead
  if (data.useCount == 0) {
    // Free the register
    assert(data.hasAddress() && "Variable doesn't have an address!");
    markRegisterAsFreed(data.addr.getValue());
    // Remove the entry from the map
    knownVars_.erase(it);
  }
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