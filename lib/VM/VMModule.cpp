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

void VMModule::setInstrs(std::unique_ptr<InstructionBuffer> buffer) {
  instrBuffer_ = std::move(buffer);
}

InstructionBuffer* VMModule::getInstrs() {
  return instrBuffer_.get();
}

const InstructionBuffer* VMModule::getInstrs() const {
  return instrBuffer_.get();
}

std::unique_ptr<InstructionBuffer> VMModule::takeInstrs() {
  return std::move(instrBuffer_);
}
