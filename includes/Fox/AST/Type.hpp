//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Type.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the Type & TypeLoc classes
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/Common/SourceLoc.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/AST/ASTAligns.hpp"
#include <iosfwd>

namespace fox {
  class TypeBase;
  class TypeLoc;
  // The Type class is an observing pointer to a TypeBase*
  //
  // This design comes from the Swift compiler. I've chosen to adopt it too
  // because I'd like to add typealiases to Fox one day (since it's pretty
  // handy with function types, which I plan to add too), so I'll need
  // to have a concept of canonical and sugared type to perform
  // typechecking properly while still emitting good diagnostics.
  // This class will be incredibely helpful because I can disable non-canonical
  // type comparison just by removing the operator== (and refactoring the code
  // that it broke)
  class Type {
    TypeBase* ty_ = nullptr;
    public:
      Type(TypeBase* ty = nullptr);

      TypeBase* getPtr();
      const TypeBase* getPtr() const;

      bool isNull() const;

      void dump() const;

      TypeBase* operator->();
      const TypeBase* operator->() const;

      explicit operator bool() const;

      bool operator==(const Type& type) const;
      bool operator!=(const Type& type) const;

      // for STL containers
      bool operator<(const Type other) const;
  };

  // A Type with its associated location information, which is
  //  stored as a SourceRange.
  class TypeLoc  {
    SourceRange range_;
    Type ty_;
    public:
      TypeLoc() = default;
      explicit TypeLoc(Type ty, SourceRange range);

      SourceRange getSourceRange() const;
      SourceLoc getBeginLoc() const;
      SourceLoc getEndLoc() const;

      // Checks that the location information is valid.
      bool isLocValid() const;
      // Checks that the type is valid
      bool isTypeValid() const;
      // Checks that both the location information & type are valid
      bool isComplete() const;

      // TypeLoc doesn't have it's own dump() method because we cannot dump
      // any meaningful information about our range_ without a SourceManager.

      // Conversion functions
      Type getType();
      const Type getType() const; 

    private:
      bool operator==(const Type& type) const = delete;
      bool operator!=(const Type& type) const = delete;
  };

  // Like SwiftC does, we'll disable isa/cast/dyn_cast/dyn_cast_or_null
  // on Type objects to eliminate bugs due to mixing Type and TypeBase*
  template <class X> inline bool isa(const Type&) = delete;
  template <class X> inline typename llvm::cast_retty<X, Type>::ret_type
    cast(const Type&) = delete;
  template <class X> inline typename llvm::cast_retty<X, Type>::ret_type
    dyn_cast(const Type&) = delete;
  template <class X> inline typename llvm::cast_retty<X, Type>::ret_type
    dyn_cast_or_null(const Type&) = delete;

  // ostream for Type class
  std::ostream& operator<<(std::ostream& os, Type ty);
}


namespace llvm {
  // A Type is just a wrapper around a TypeBase*, and thus can be considered
  // as pointer-like.
  template<>
  struct PointerLikeTypeTraits<::fox::Type> {
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