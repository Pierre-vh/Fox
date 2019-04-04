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
  return getInstructions().size();
}

InstructionVector& BCModule::getInstructions() {
  return instrs_;
}

const InstructionVector& BCModule::getInstructions() const {
  return instrs_;
}

void BCModule::dumpModule(std::ostream& out) const {
  dumpInstructions(out, getInstructions());
}

InstructionVector::iterator BCModule::instrs_begin() {
  return instrs_.begin();
}

InstructionVector::iterator BCModule::instrs_end() {
  return instrs_.end();
}