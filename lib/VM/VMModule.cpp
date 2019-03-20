//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : VMModule.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/VM/VMModule.hpp"
#include "llvm/ADT/ArrayRef.h"

using namespace fox;

std::size_t VMModule::numInstructions() const {
  return getInstructionBuffer().size();
}

InstructionBuffer& VMModule::getInstructionBuffer() {
  return instrBuffer_;
}

const InstructionBuffer& VMModule::getInstructionBuffer() const {
  return instrBuffer_;
}

void VMModule::dumpModule(std::ostream& out) const {
  dumpInstructions(out, getInstructionBuffer());
}
