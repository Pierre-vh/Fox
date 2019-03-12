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

RegisterValue RegisterAllocator::allocateNewRegister() {
  fox_unimplemented_feature("RegisterAllocator::allocateNewRegister() "
    "isn't implemented yet");
  // TODO: Check if it's correct, if no other operation are needed.
  
  // return RegisterValue(this, rawAllocateNewRegister());
}

regnum_t RegisterAllocator::rawAllocateNewRegister() {
  fox_unimplemented_feature("RegisterAllocator::rawAllocateNewRegister() "
    "isn't implemented yet");
  // Pick a register in freedRegs if it's not empty, else
  // pick highestReg++

  // When we can't allocate a new registers (all regs are occupied)
  // just assert and I'll add precise diagnostisc later when Functions 
  // are actually being compiled.
  //
  // When functions are compiled, I can emit a diagnostic (error), like
  // Function 'foo' to complex to be compiled:
  // function requires too many registers to be compiled'
}

void RegisterAllocator::markRegisterAsFreed(regnum_t /*reg*/) {
  // Mark the register as freed:
  //  if (reg+1 == highestReg) highestReg--;
  //  else freedRegs.insert(reg);
  fox_unimplemented_feature("RegisterAllocator::markRegisterAsFreed() "
    "isn't implemented yet");
}

//----------------------------------------------------------------------------//
// RegisterValue
//----------------------------------------------------------------------------//

RegisterValue::RegisterValue(RegisterAllocator* regAlloc, regnum_t reg) : 
  regAlloc_(regAlloc), regNum_(reg) {}

RegisterValue::RegisterValue(RegisterValue&& other) {
  (*this) = std::move(other);
}

RegisterValue::~RegisterValue() {
  free();
}

RegisterValue& RegisterValue::operator=(RegisterValue&& other) {
  free();
  regAlloc_ = other.regAlloc_;
  regNum_ = other.regNum_;
  other.kill();
  return *this;
}

regnum_t RegisterValue::getRegisterNumber() const {
  return regNum_;
}

bool RegisterValue::isAlive() const {
  // We're alive if our RegisterAllocator* is non null.
  return (bool)regAlloc_;
}

void RegisterValue::free() {
  // Can't free a dead RegisterValue
  if(!isAlive()) return;
  // Free our register and kill this object so the
  // register is not freed again by mistake.
  regAlloc_->markRegisterAsFreed(regNum_);
  kill();
}

void RegisterValue::kill() {
  regAlloc_ = nullptr;
}