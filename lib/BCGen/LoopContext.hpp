//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : LoopContext.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the LoopContext class.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/BC/BCUtils.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"
#include <unordered_set>

namespace fox {
  class RegisterAllocator;
  class VarDecl;
  // The LoopContext is a data structure that should be used when
  // compiling loops.
  // It is a very important part of the codegen process for loops because
  // It prevents variables declared outside the loop from dying inside it.
  class LoopContext {
    public:
      LoopContext(RegisterAllocator& regAlloc);
      ~LoopContext();
      
      RegisterAllocator& regAlloc;
    private:
      friend RegisterAllocator;

      // Returns true if 'var' was declared inside this loop
      bool isVarDeclaredInside(const VarDecl* var) const;

      // The set of variables declared inside this loop. 
      // These are the only variables that can be destroyed when inside
      // the loop.
      std::unordered_set<const VarDecl*> varsInLoop_;
      // The set of variables declared outside the loop that 'died'
      // while inside the loop. 
      std::unordered_set<const VarDecl*> delayedFrees_;
      LoopContext* previousLC_= nullptr;
  };
}