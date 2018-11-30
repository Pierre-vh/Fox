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
#include <iosfwd>

namespace fox {
  class TypeBase;
  class TypeLoc;

  // Type class, an observing pointer to a TypeBase*.
  // Never use raw TypeBase pointers unless necessary, always use this wrapper.
  // Used to facilitate passing TypeBase pointers as reference
  //    e.g. Type& instead of TypeBase*&
  // + it adds flexibility in case I'd like to add "sugared" types one day.
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

    private:
      // Forbid TypeLoc->Type conversion
      Type(const TypeLoc&) = delete;
      Type& operator=(const TypeLoc&) = delete;
  };

  // A Type with its associated SourceRange
  class TypeLoc : public Type {
    SourceRange range_;
    public:
      TypeLoc(TypeBase* ty = nullptr, SourceRange range = SourceRange());
      TypeLoc(Type ty, SourceRange range = SourceRange());

      SourceRange getRange() const;

      // TypeLoc doesn't have it's own dump() method because we cannot dump
      // any meaningful information about our range_ without a SourceManager.

      Type withoutLoc();
      const Type withoutLoc() const; 

    private:
      // For now, disable TypeLoc comparison. We don't need it.
      // Might add a "strict_compare" function tho.
      bool operator==(const Type& type) const = delete;
      bool operator!=(const Type& type) const = delete;

      // Forbid Type->TypeLoc conversion
      TypeLoc(const Type&) = delete;
      TypeLoc& operator=(const Type&) = delete;
  };

  // Like SwiftC does, we'll disable isa/cast/dyn_cast/dyn_cast_or_null
  // on Type objects to eliminate bugs due to mixing Type and TypeBase*
  template <class X> 
  inline bool isa(const Type&) = delete; 

  template <class X> inline typename llvm::cast_retty<X, Type>::ret_type 
  cast(const Type&) = delete;

  template <class X> inline typename llvm::cast_retty<X, Type>::ret_type 
  dyn_cast(const Type&) = delete;

  template <class X> inline typename llvm::cast_retty<X, Type>::ret_type 
  dyn_cast_or_null(const Type&) = delete;

  // ostream for Type class
  std::ostream& operator<<(std::ostream& os, Type ty);
}
