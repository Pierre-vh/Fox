//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BCModule.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BC/BCModule.hpp"
#include "llvm/ADT/ArrayRef.h"

using namespace fox;

std::size_t BCModule::numInstructions() const {
  return getInstructionBuffer().size();
}

InstructionBuffer& BCModule::getInstructionBuffer() {
  return instrBuffer_;
}

const InstructionBuffer& BCModule::getInstructionBuffer() const {
  return instrBuffer_;
}

void BCModule::dumpModule(std::ostream& out) const {
  dumpInstructions(out, getInstructionBuffer());
}
