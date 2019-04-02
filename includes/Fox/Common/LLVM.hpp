//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : LLVM.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file imports/forward declares some commonly used LLVM classes/functions 
// that we want to use unqualified.
//----------------------------------------------------------------------------//

// We need to import some classes because they can't be easily forward
// declared
#include "llvm/Support/Casting.h" // complex templates
#include "llvm/ADT/None.h"        // can't forward declare

namespace llvm {
  template <typename T> class SmallVectorImpl;
  template <typename T, unsigned N> class SmallVector;
  template<typename T> class ArrayRef;
  template<typename T> class Optional;
  template<typename T> class MutableArrayRef;
}

namespace fox {
  using llvm::isa;
  using llvm::cast;
  using llvm::dyn_cast;
  using llvm::dyn_cast_or_null;
  using llvm::cast_or_null;

  using llvm::SmallVectorImpl;
  using llvm::SmallVector;

  using llvm::Optional;
  using llvm::None;

  using llvm::ArrayRef;
  using llvm::MutableArrayRef;
}