//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : ASTAligns.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains various constants to be used as the minimum
// alignement for some AST classes
//----------------------------------------------------------------------------//

#pragma once

#include <cstddef>
#include "ASTFwdDecl.hpp"

namespace fox {
namespace align {
  // Declare the FreeBits and Alignement variables
  // Usage of DECLARE: DECLARE(Class name, Number of free bits desired)
  #define DECLARE(CLASS, FREE_BITS_DESIRED)\
  constexpr std::size_t CLASS##FreeLowBits = FREE_BITS_DESIRED; \
  constexpr std::size_t CLASS##Alignement = 1 << FREE_BITS_DESIRED

  DECLARE(TypeBase, 1);
  DECLARE(Expr, 2);
  DECLARE(Decl, 2);
  DECLARE(Stmt, 2);
  DECLARE(DeclContext, 2);
  #undef DECLARE
}
}

// Specialize llvm::PointerLikeTypeTraits for each class.
// This is important for multiple LLVM ADT classes, such as
// PointerUnion
namespace llvm {
  template <class T> struct PointerLikeTypeTraits;
  #define LLVM_DEFINE_PLTT(CLASS) \
  template <> struct PointerLikeTypeTraits<::fox::CLASS*> { \
    enum { NumLowBitsAvailable = ::fox::align::CLASS##FreeLowBits }; \
    static inline void* getAsVoidPointer(::fox::CLASS* ptr) {return ptr;} \
    static inline ::fox::CLASS* getFromVoidPointer(void* ptr) \
    {return static_cast<::fox::CLASS*>(ptr);} \
  }

  LLVM_DEFINE_PLTT(TypeBase);
  LLVM_DEFINE_PLTT(Expr);
  LLVM_DEFINE_PLTT(Decl);
  LLVM_DEFINE_PLTT(Stmt);
  LLVM_DEFINE_PLTT(DeclContext);

  #undef LLVM_DEFINE_PLTT

  // For PointerLikeTypeTraits
  static_assert(alignof(void*) >= 2, "void* pointer alignment too small");
}

