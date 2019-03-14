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
  assert((biggestAllocatedReg_ != maxAddr) && 
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

RegisterValue::RegisterValue(RegisterAllocator* regAlloc, regaddr_t reg) : 
  regAlloc_(regAlloc), regAddress_(reg) {}

RegisterValue::RegisterValue(RegisterValue&& other) {
  (*this) = std::move(other);
}

RegisterValue::~RegisterValue() {
  free();
}

RegisterValue& RegisterValue::operator=(RegisterValue&& other) {
  free();
  regAlloc_ = std::move(other.regAlloc_);
  regAddress_   = std::move(other.regAddress_);
  other.kill();
  return *this;
}

regaddr_t RegisterValue::getAddress() const {
  return regAddress_;
}

bool RegisterValue::isAlive() const {
  // We're alive if our RegisterAllocator* is non null.
  return (bool)regAlloc_;
}

bool RegisterValue::isTemporary() const {
  // There are only temp RegisterValues for now.
  return true;
}

void RegisterValue::free() {
  // Can't free a dead RegisterValue
  if(!isAlive()) return;
  // Free our register and kill this object so the
  // register is not freed again by mistake.
  regAlloc_->markRegisterAsFreed(regAddress_);
  kill();
}

void RegisterValue::kill() {
  regAlloc_ = nullptr;
}