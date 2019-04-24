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

BCFunction::BCFunction(std::size_t id) : id_(id) {}

BCFunction::BCFunction(std::size_t id, ParamCopyMap paramCopyMap)
  : id_(id), paramCopyMap_(paramCopyMap) {
  needsCopyAfterReturn_ = paramCopyMap_.any();
}

std::size_t BCFunction::getID() const {
  return id_;
}

std::size_t BCFunction::numInstructions() const {
  return instrs_.size();
}

BCBuilder BCFunction::createBCBuilder() {
  return BCBuilder(instrs_);
}

InstructionVector& BCFunction::getInstructions() {
  return instrs_;
}

const InstructionVector& BCFunction::getInstructions() const {
  return instrs_;
}

const BCFunction::ParamCopyMap& BCFunction::getParamCopyMap() const {
  return paramCopyMap_;
}

bool BCFunction::needsCopyAfterReturn() const {
  return needsCopyAfterReturn_;
}

void BCFunction::dump(std::ostream& out) const {
  out << "Function " << id_ << "\n";
  // Dump the paramCopyMap_ if it contains something
  if (paramCopyMap_.size()) {
    out << "  PCM: ";

    std::size_t pcm_size = paramCopyMap_.size();
    bool first = true;
    for (std::size_t idx = 0; idx < pcm_size; ++idx) {
      if(first) first = false;
      else out << "-";
      out << paramCopyMap_[idx] ? "1" : "0";
    }

    out << '\n';
  }

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