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
  class ValueDecl;
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

      // Returns true if \p decl was declared inside this loop
      bool isDeclaredInside(const ValueDecl* decl) const;

      // The set of decls declared inside this loop. 
      // These are the only Decls that can be destroyed when their
      // use count reaches zero while we are in this LoopContext.
      std::unordered_set<const ValueDecl*> declsInLoop_;
      // The set of Decls declared outside the loop that 'died'
      // while inside the loop. 
      std::unordered_set<const ValueDecl*> delayedReleases_;
      LoopContext* previousLC_= nullptr;
  };
}