//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : LLVM.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file imports the most commonly used LLVM functions that
// we want to use unqualified.
//----------------------------------------------------------------------------//

// Include Casting.h because it has complex templated functions
// that can't be easily forward-declared. 
#include "llvm/Support/Casting.h"

// Forward-declare SmallVector 
namespace llvm {
  template <typename T> class SmallVectorImpl;
  template <typename T, unsigned N> class SmallVector;
}

namespace fox {
  using llvm::isa;
  using llvm::cast;
  using llvm::dyn_cast;
  using llvm::dyn_cast_or_null;
  using llvm::cast_or_null;

  using llvm::SmallVectorImpl;
  using llvm::SmallVector;
}