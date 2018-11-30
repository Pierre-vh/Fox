//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Types.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the TypeBase hierarchy.
//
//  Unlike Stmt/Decl/Expr hierarchies, this hierarchy is exclusively
//  allocated/created through static member functions. This is done because
//  most types are unique.
//
//----------------------------------------------------------------------------//

#pragma once

#include <string>
#include <cstdint>
#include <list>
#include "llvm/ADT/PointerIntPair.h"
#include "ASTAligns.hpp"

namespace fox {
  // Kinds of Types
  enum class TypeKind : std::uint8_t {
    #define TYPE(ID,PARENT) ID,
    #define TYPE_RANGE(ID,FIRST,LAST) First_##ID = FIRST, Last_##ID = LAST,
    #include "TypeNodes.def"
  };

  // Forward Declarations
  class ASTContext;

  // TypeBase
  //    Common base for types
  class alignas(align::TypeBaseAlignement) TypeBase {
    public:
      /* Returns the type's name in a user friendly form, e.g. "int", "string" */
      std::string toString() const;

      /* Returns the type's name in a developer-friendly form, e.g. "Array(int)" instead of [int]" */
      std::string toDebugString() const;

      void dump() const;

      TypeKind getKind() const;

      // Returns the element type if this is an ArrayType, or nullptr.
      const TypeBase* unwrapIfArray() const;
      TypeBase* unwrapIfArray();

      // Returns the element type if this is an LValueType, or nullptr.
      const TypeBase* unwrapIfLValue() const;
      TypeBase* unwrapIfLValue();

      // If this type is an LValue, returns it's element type, else
      // returns this.
      const TypeBase* ignoreLValue() const;
      TypeBase* ignoreLValue();

      bool isStringType() const;
      bool isCharType() const;
      bool isFloatType() const;
      bool isBoolType() const;
      bool isIntType() const;
      bool isVoidType() const;

      template<typename Ty>
      bool is() const {
        return isa<Ty>(this);
      }

      template<typename Ty>
      const Ty* getAs() const {
        return dyn_cast<Ty>(this);
      }

      template<typename Ty>
      Ty* getAs() {
        return dyn_cast<Ty>(this);
      }

    protected:
      TypeBase(TypeKind tc);

      // Prohibit the use of builtin placement new & delete
      void *operator new(std::size_t) throw() = delete;
      void operator delete(void *) throw() = delete;
      void* operator new(std::size_t, void*) = delete;

      // Only allow allocation through the ASTContext
      // This operator is "protected" so only the ASTContext can create types.
      void* operator new(std::size_t sz, ASTContext &ctxt, 
      std::uint8_t align = alignof(TypeBase));

      // Companion operator delete to silence C4291 on MSVC
      void operator delete(void*, ASTContext&, std::uint8_t) {}
    
    private:
      const TypeKind kind_;
  };

  // BasicType
  //    Common base for "Basic" Types.
  //    A basic type is a type that can't be unwrapped any further.
  //    Every type is Fox is made of 1 or more Basic type used in conjuction
  //    with other types, such as the LValueType or the ArrayType.
  //  
  class BasicType : public TypeBase {
    public:
      static bool classof(const TypeBase* type) {
        return ((type->getKind() >= TypeKind::First_BasicType) 
          && (type->getKind() <= TypeKind::Last_BasicType));
      }

    protected:
      BasicType(TypeKind tc);
  };

  // PrimitiveType 
  //    A primitive type (void/int/float/char/bool/string)
  class PrimitiveType : public BasicType {
    public:
      enum class Kind : std::uint8_t {
        VoidTy,
        IntTy,
        FloatTy,
        CharTy,
        StringTy,
        BoolTy
      };

      static PrimitiveType* getString(ASTContext& ctxt);
      static PrimitiveType* getChar(ASTContext& ctxt);
      static PrimitiveType* getFloat(ASTContext& ctxt);
      static PrimitiveType* getBool(ASTContext& ctxt);
      static PrimitiveType* getInt(ASTContext& ctxt);
      static PrimitiveType* getVoid(ASTContext& ctxt);

      Kind getPrimitiveKind() const;

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::PrimitiveType);
      }

    private:
      PrimitiveType(Kind kd);

      const Kind builtinKind_;
  };

 // ErrorType
  //    A type used to represent that a expression's type
  //    cannot be determined because of an error.
  class ErrorType : public BasicType {
    public:
      // Gets the unique ErrorType instance for the current context.
      static ErrorType* get(ASTContext& ctxt);

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::ErrorType);
      }

    private:
      ErrorType();
  };

  // ArrayType
  //    An array of a certain type (can be any type, 
  //    even another ArrayType)
  class ArrayType : public TypeBase {
    public:
      // Returns the UNIQUE ArrayType instance for the given
      // type ty.
      static ArrayType* get(ASTContext& ctxt, TypeBase* ty);

      TypeBase* getElementType();
      const TypeBase* getElementType() const;

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::ArrayType);
      }

    private:
      // Private because only called by ::get
      ArrayType(TypeBase* elemTy);

      TypeBase* elementTy_= nullptr;
  };

  // LValueType
  //    C/C++-like LValue. e.g. This type is the one
  //    of a DeclRef when the declaration it refers to
  //    is not const.
  class LValueType : public TypeBase {
    public:
      // Returns the UNIQUE LValueType instance for the given type "ty"
      static LValueType* get(ASTContext& ctxt, TypeBase* ty);

      TypeBase* getType();
      const TypeBase* getType() const;

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::LValueType);
      }

    private:
      // Private because only used by ::get
      LValueType(TypeBase* type);

      TypeBase* ty_ = nullptr;
  };

  // CellType
  class CellType : public TypeBase {
    public:
      // Creates a new instance of the CellType class
      static CellType* create(ASTContext& ctxt);

      TypeBase* getSubstitution();
      const TypeBase* getSubstitution() const;

      // Returns true if the type has a substitution
      // (type isn't null)
      bool hasSubstitution() const;

      void setSubstitution(TypeBase* type);

      void reset();

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::CellType);
      }

    private:
      // Private because only called by ::create
      CellType();

      // Override the new operator to use the SemaAllocator in the
      // ASTContext to allocate CellTypes.
      void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(TypeBase));

      TypeBase* type_ = nullptr;
  };

}
