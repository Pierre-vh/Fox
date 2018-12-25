//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Type.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the Type & TypeLoc classes
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/Common/Source.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/AST/ASTAligns.hpp"
#include <iosfwd>

namespace fox {
  class TypeBase;
  class TypeLoc;
  // The Type class is an observing pointer to a TypeBase*.
  // 
  // This is used for multiple reasons, but mostly for future proofing.
  // This will be of invaluable use when/if I add sugared types to Fox in
  // the future, as it'll allow me to disable type comparison easily.
  class Type {
    TypeBase* ty_ = nullptr;
    public:
      Type(TypeBase* ty = nullptr);

      TypeBase* getPtr() const;

      bool isNull() const;

      void dump() const;

      TypeBase* operator->();
      const TypeBase* operator->() const;

      explicit operator bool() const;

      bool operator==(const Type& type) const;
      bool operator!=(const Type& type) const;

      // for STL containers
      bool operator<(const Type other) const;

    private:
      // Disable TypeLoc->Type conversion
      Type(const TypeLoc&) = delete;
      Type& operator=(const TypeLoc&) = delete;
  };

  // A Type with its associated location information (a SourceRange)
  class TypeLoc : public Type {
    SourceRange range_;
    public:
      TypeLoc(TypeBase* ty = nullptr, SourceRange range = SourceRange());
      TypeLoc(Type ty, SourceRange range = SourceRange());

      SourceRange getRange() const;
      SourceLoc getBegin() const;
      SourceLoc getEnd() const;

      // TypeLoc doesn't have it's own dump() method because we cannot dump
      // any meaningful information about our range_ without a SourceManager.

      // Conversion functions
      Type withoutLoc();
      const Type withoutLoc() const; 

    private:
      // For now, disable TypeLoc comparison. We don't need it.
      // Might add a "strict_compare" function tho.
      bool operator==(const Type& type) const = delete;
      bool operator!=(const Type& type) const = delete;

      // Disable Type->TypeLoc conversion
      TypeLoc(const Type&) = delete;
      TypeLoc& operator=(const Type&) = delete;
  };

  // Like SwiftC does, we'll disable isa/cast/dyn_cast/dyn_cast_or_null
  // on Type objects to eliminate bugs due to mixing Type and TypeBase*
#define DISABLE_LLVM_RTTI_FUNCS(CLASS)\
  template <class X> inline bool isa(const CLASS&) = delete;               \
  template <class X> inline typename llvm::cast_retty<X, CLASS>::ret_type  \
    cast(const CLASS&) = delete;                                           \
  template <class X> inline typename llvm::cast_retty<X, CLASS>::ret_type  \
    dyn_cast(const CLASS&) = delete;                                       \
  template <class X> inline typename llvm::cast_retty<X, CLASS>::ret_type  \
    dyn_cast_or_null(const CLASS&) = delete 

  DISABLE_LLVM_RTTI_FUNCS(Type);
  DISABLE_LLVM_RTTI_FUNCS(TypeLoc);
#undef DISABLE_LLVM_RTTI_FUNCS

  // ostream for Type class
  std::ostream& operator<<(std::ostream& os, Type ty);
}


namespace llvm {
  // A Type is just a wrapper around a TypeBase*, and thus can be considered
  // as pointer-like.
  template<>
  class PointerLikeTypeTraits<::fox::Type> {
    public:
      enum { NumLowBitsAvailable = ::fox::TypeBaseFreeLowBits };

      static inline void* getAsVoidPointer(::fox::Type type) {
        return type.getPtr();
      }

      static inline ::fox::Type getFromVoidPointer(void* ptr) {
        return (::fox::TypeBase*)ptr;
      }
  };
}