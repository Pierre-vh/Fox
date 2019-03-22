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

void BCModule::erase(instr_iterator beg, instr_iterator end) {
  auto true_beg = beg.toIBiterator();
  auto true_end = end.toIBiterator();
  instrBuffer_.erase(true_beg, true_end);
}

void BCModule::popInstr() {
  instrBuffer_.pop_back();
}

BCModule::instr_iterator BCModule::instrs_begin() {
  return instr_iterator(*this, 0);
}

BCModule::instr_iterator BCModule::instrs_end() {
  return instr_iterator(*this, instrBuffer_.size());
}

BCModule::instr_iterator BCModule::instrs_back() {
  return instr_iterator(*this, instrBuffer_.size()-1);
}

BCModule::instr_iterator BCModule::addInstr(Instruction instr) {
  instrBuffer_.push_back(instr);
  return instrs_back();
}

//----------------------------------------------------------------------------//
// BCModule::instr_iterator
//----------------------------------------------------------------------------//

BCModule::instr_iterator& 
BCModule::instr_iterator::operator=(const BCModule::instr_iterator& other) {
  bcModule_ = other.bcModule_;
  idx_ = other.idx_;
  return *this;
}

BCModule::instr_iterator& BCModule::instr_iterator::operator++() {
  assert((idx_ < bcModule_.get().getInstructionBuffer().size()) 
    && "Incrementing a past-the-end iterator");
  ++idx_;
  return *this;
}

BCModule::instr_iterator BCModule::instr_iterator::operator++(int) {
  auto save = (*this);
  ++(*this);
  return save;
}

BCModule::instr_iterator& BCModule::instr_iterator::operator--() {
  assert((idx_ != 0) 
    && "Decrementing a begin iterator");
  --idx_;
  return *this;
}

BCModule::instr_iterator BCModule::instr_iterator::operator--(int) {
  auto save = (*this);
  --(*this);
  return save;
}

BCModule::instr_iterator::reference_type 
BCModule::instr_iterator::operator*() const {
  return get();
}

BCModule::instr_iterator::pointer_type 
BCModule::instr_iterator::operator->() const {
  return &(get());
}

bool 
fox::operator==(const BCModule::instr_iterator& lhs, 
               const BCModule::instr_iterator& rhs) {
  // Check if the modules are the same (same instance)
  if(lhs.usesSameModuleAs(rhs))
    return lhs.idx_ == rhs.idx_;
  return false;
}

bool 
fox::operator!=(const BCModule::instr_iterator& lhs, 
               const BCModule::instr_iterator& rhs) {
  return !(lhs == rhs);
}

bool 
fox::operator<(const BCModule::instr_iterator& lhs, 
               const BCModule::instr_iterator& rhs) {
  assert(lhs.usesSameModuleAs(rhs) 
    && "iterators are for different BCModules!");
  return lhs.idx_ < rhs.idx_;
}

bool 
fox::operator>(const BCModule::instr_iterator& lhs, 
               const BCModule::instr_iterator& rhs) {
  assert(lhs.usesSameModuleAs(rhs) 
    && "iterators are for different BCModules!");
  return lhs.idx_ > rhs.idx_;
}

BCModule::instr_iterator::difference_type 
fox::distance(BCModule::instr_iterator first, BCModule::instr_iterator last) {
  // First assert that they both share the same module* and that
  // first < last.
  assert(first.usesSameModuleAs(last) 
    && "iterators are for different BCModules!");
  assert((first.idx_ < last.idx_) 
    && "last > first!");
  return (last.idx_ - first.idx_);
}

InstructionBuffer::iterator BCModule::instr_iterator::toIBiterator() const {
  return bcModule_.get().instrBuffer_.begin() + idx_;
}

bool
BCModule::instr_iterator::usesSameModuleAs(const instr_iterator& other) const {
  return &(bcModule_.get()) == &(other.bcModule_.get());
}

BCModule::instr_iterator::reference_type BCModule::instr_iterator::get() const {
  auto& iBuff = bcModule_.get().instrBuffer_;
  assert(idx_ < iBuff.size() 
    && "Dereferencing past-the-end iterator");
  return iBuff[idx_];
}

BCModule::instr_iterator::instr_iterator(BCModule& bcModule, idx_type idx)
  : bcModule_(bcModule), idx_(idx) {}