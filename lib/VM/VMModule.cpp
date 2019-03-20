//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : VMModule.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/VM/VMModule.hpp"
#include "Fox/VM/Instructions.hpp"

using namespace fox;

// Theses have to be defined here because the header only forward-declares
// the Instruction struct.
VMModule::VMModule() = default;
VMModule::~VMModule() = default;

void VMModule::setInstructionBuffer(std::unique_ptr<InstructionBuffer> buffer) {
  instrBuffer_ = std::move(buffer);
}

ArrayRef<InstructionBuffer> fox::VMModule::getInstructionBuffer() const {
  return (*instrBuffer_);
}

std::unique_ptr<InstructionBuffer> VMModule::takeInstructionBuffer() {
  return std::move(instrBuffer_);
}
