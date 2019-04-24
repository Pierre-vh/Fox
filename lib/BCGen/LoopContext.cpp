//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : LoopContext.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "LoopContext.hpp"
#include "Registers.hpp"

using namespace fox;

LoopContext::LoopContext(RegisterAllocator& regAlloc) : regAlloc(regAlloc) {
  // TOOD: Abstract this in a "RegisterAllocator::actOnNewLoopContext" method
  previousLC_ = regAlloc.curLoopContext_;
  regAlloc.curLoopContext_ = this;
}

LoopContext::~LoopContext() {
  // Restore the previous LC
  regAlloc.curLoopContext_ = previousLC_;
  // Notify the RegisterAllocator
  regAlloc.actOnEndOfLoopContext(*this);
}

bool LoopContext::isDeclaredInside(const ValueDecl* decl) const {
  return (declsInLoop_.find(decl) != declsInLoop_.end());
}
