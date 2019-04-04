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

InstructionBuffer& BCModule::getInstructions() {
  return instrBuffer_;
}

const InstructionBuffer& BCModule::getInstructions() const {
  return instrBuffer_;
}

void BCModule::dumpModule(std::ostream& out) const {
  dumpInstructions(out, getInstructions());
}

BCModule::instr_iterator BCModule::instrs_begin() {
  return instr_iterator(getInstructions());
}

BCModule::instr_iterator BCModule::instrs_end() {
  return instr_iterator(getInstructions(), instrBuffer_.size());
}

BCModule::instr_iterator BCModule::instrs_last() {
  return instr_iterator(getInstructions(), instrBuffer_.size()-1);
}

BCModule::instr_iterator BCModule::addInstr(Instruction instr) {
  instrBuffer_.push_back(instr);
  return instrs_last();
}