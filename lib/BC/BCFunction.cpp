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

BCBuilder BCFunction::createBCBuilder() {
  return BCBuilder(instrs_, debugInfo_.get());
}

void BCFunction::dump(std::ostream& out, string_view title) const {
  out << title << ' ' << id_ << '\n';

  if(instrs_.empty())
    out << "    <empty>\n";
  else
    dumpInstructions(out, instrs_, "   ");
}

DebugInfo& BCFunction::createDebugInfo() {
  assert(!hasDebugInfo()
    && "already has debug info");
  debugInfo_ = std::make_unique<DebugInfo>();
  return *debugInfo_;
}

void BCFunction::removeDebugInfo() {
  assert(hasDebugInfo() 
    && "can't remove debug info if there is no debug info!");
}

bool BCFunction::hasDebugInfo() const {
  return (bool)debugInfo_;
}

DebugInfo* BCFunction::getDebugInfo() {
  return debugInfo_.get();
}

const DebugInfo* BCFunction::getDebugInfo() const {
  return debugInfo_.get();
}
