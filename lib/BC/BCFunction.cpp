//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BCFunction.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BC/BCFunction.hpp"
#include "Fox/BC/BCBuilder.hpp"
#include "llvm/ADT/ArrayRef.h"

using namespace fox;

BCFunction::BCFunction(std::size_t id, std::size_t numParams) 
  : id_(id), numParams_(numParams) {}

std::size_t BCFunction::numInstructions() const {
  return instrs_.size();
}

BCBuilder BCFunction::createBCBuilder() {
  return BCBuilder(instrs_);
}

void BCFunction::dump(std::ostream& out) const {
  out << "Function " << id_ << "\n";

  if(instrs_.empty())
    out << "    <empty>\n";
  else
  dumpInstructions(out, instrs_, "   ");
}

InstructionVector::iterator BCFunction::instrs_begin() {
  return instrs_.begin();
}

InstructionVector::const_iterator BCFunction::instrs_begin() const {
  return instrs_.begin();
}

InstructionVector::iterator BCFunction::instrs_end() {
  return instrs_.end();
}

InstructionVector::const_iterator BCFunction::instrs_end() const {
  return instrs_.end();
}