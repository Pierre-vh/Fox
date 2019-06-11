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
  return BCBuilder(instrs_);
}

void BCFunction::dump(std::ostream& out, string_view title) const {
  out << title << ' ' << id_ << '\n';

  if(instrs_.empty())
    out << "    <empty>\n";
  else
    dumpInstructions(out, instrs_, "   ");
}