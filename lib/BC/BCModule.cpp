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

//----------------------------------------------------------------------------//
// BCModule::instr_iterator
//----------------------------------------------------------------------------//

BCModule::instr_iterator& 
BCModule::instr_iterator::operator++() {
  idx_++;
  return *this;
}

BCModule::instr_iterator
BCModule::instr_iterator::operator++(int) {
  auto save = (*this);
  ++(*this);
  return save;
}

BCModule::instr_iterator::reference_type 
BCModule::instr_iterator::operator*() const {
  return get();
}

BCModule::instr_iterator::reference_type 
BCModule::instr_iterator::operator->() const {
  return get();
}

bool fox::operator==(BCModule::instr_iterator lhs, 
                     BCModule::instr_iterator rhs){
  // Check if the modules are the same (same instance)
  if(&(lhs.bcModule_) == &(rhs.bcModule_))
    return lhs.idx_ == rhs.idx_;
  return false;
}

bool fox::operator!=(BCModule::instr_iterator lhs, 
                     BCModule::instr_iterator rhs){
  return !(lhs == rhs);
}

BCModule::instr_iterator::reference_type
BCModule::instr_iterator::get() {
  auto& iBuff = bcModule_.instrBuffer_;
  assert(idx_ < iBuff.size() 
    && "Dereferencing past-the-end iterator");
  return iBuff[idx_];
}

const BCModule::instr_iterator::reference_type 
BCModule::instr_iterator::get() const {
  return const_cast<instr_iterator*>(this)->get();
}

BCModule::instr_iterator::instr_iterator(BCModule& bcModule, idx_type idx)
  : bcModule_(bcModule), idx_(idx) {}