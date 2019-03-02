//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : VM.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/VM/VM.hpp"

using namespace fox;

void VM::load(ArrayRef<std::uint32_t> instrs) {
  program_ = instrs;
  programCounter_ = 0;
}

void VM::run() {
  // TODO
}

ArrayRef<std::uint64_t> VM::getRegisterStack() const {
  return regStack_;
}

MutableArrayRef<std::uint64_t> VM::getRegisterStack() {
  return regStack_;
}
