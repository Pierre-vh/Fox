//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Types.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the TypeBase hierarchy.
//----------------------------------------------------------------------------//

#pragma once

#include "ASTAligns.hpp"
#include "Type.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/TrailingObjects.h"
#include "llvm/ADT/ArrayRef.h"
#include <string>
#include <cstdint>

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
  //
  // Usually, you should never use raw TypeBase* pointers unless you have
  // a valid reason. Always use the Type wrapper. (see Type.hpp)
  class alignas(TypeBaseAlignement) TypeBase {
    public:
      // Returns the type's name in a user friendly form, 
      //   e.g. "int", "string"
      std::string toString() const;

      // Returns the type's name in a more "developer-friendly"
      // form, which provides more information.
      //   e.g. "Array(int)" instead of [int]"
      std::string toDebugString() const;

      void dump() const;

      TypeKind getKind() const;

      // Returns true if this is a bound type.
      //
      // A bound type is a type that doesn't contain
      // CellTypes with no substitution somewhere in it's hierarchy.
      bool isBound() const;

      // Returns the element type if this is an ArrayType, otherwise returns
      // nullptr.
      Type unwrapIfArray();

      // If this type is an LValue, returns it's element type, else
      // returns this.
      Type getRValue();

      // If this is a bound type, ignores LValues and dereferences this type. 
      // If this is an unbound type, returns nullptr.
      Type getAsBoundRValue();

      // If this type is a CellType, dereference it recursively 
      // until we reach a CellType with no substitution or a
      // concrete type.
      Type deref();

      /*
        A special note about the is/getAs/castTo
        family of function : they're strictly helpers.
        They won't see through LValues, CellTypes, Arrays, etc.

        For instance :
          (int)->isIntType() returns true
          Cell(int)->isIntType() returns false
          LValue(int)->isIntType() returns false
      */

      //-------------------------//
      // Type categories
      //-------------------------//

      bool isStringType() const;
      bool isCharType() const;
      bool isDoubleType() const;
      bool isBoolType() const;
      bool isIntType() const;
      bool isVoidType() const;
      bool isNumeric() const;
      bool isNumericOrBool() const;

      // Return true if this type can appear on the LHS of an assignement.
      bool isAssignable() const;

      template<typename Ty>
      bool is() const {
        return isa<Ty>(this);
      }

      template<typename Ty>
      Ty* getAs() {
        return dyn_cast<Ty>(this);
      }

      template<typename Ty>
      const Ty* getAs() const {
        return dyn_cast<Ty>(this);
      }

      template<typename Ty>
      Ty* castTo() {
        return cast<Ty>(this);
      }

      template<typename Ty>
      const Ty* castTo() const {
        return cast<Ty>(this);
      }

      //-------------------------//

    protected:
      TypeBase(TypeKind tc);

      // Prohibit the use of builtin placement new & delete
      void *operator new(std::size_t) throw() = delete;
      void operator delete(void *) throw() = delete;

      // Only allow allocation through the ASTContext
      void* operator new(std::size_t sz, ASTContext &ctxt, 
      std::uint8_t align = alignof(TypeBase));

      // And through placement new
      void* operator new(std::size_t, void* buff);

    private:
      void initBitfields();

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
  class PrimitiveType final : public BasicType {
    public:
      enum class Kind : std::uint8_t {
        VoidTy,
        IntTy,
        DoubleTy,
        CharTy,
        StringTy,
        BoolTy
      };

      static PrimitiveType* getString(ASTContext& ctxt);
      static PrimitiveType* getChar(ASTContext& ctxt);
      static PrimitiveType* getDouble(ASTContext& ctxt);
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
  class ErrorType final : public BasicType {
    public:
      // Gets the unique ErrorType instance for the current context.
      static ErrorType* get(ASTContext& ctxt);

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::ErrorType);
      }

    private:
      ErrorType();
  };

  // FunctionType
  //    Represents the type of a function. 
  //    Example: (int, int) -> int
  //
  //  Note: Currently, the FunctionType doesn't represent the "mut"
  //    qualifier, simply because there is no point in representing it.
  //    Why? For now, the mut qualifier is only important in 
  //    semantic analysis: It's considered when the params are pushed to the
  //    scope. If it's a mut param -> use an lvalue, otherwise don't use one.
  //    I wouldn't gain anything by representing it in types since I don't have
  //    functions as first class citizens (for now)
  class FunctionType final : public TypeBase, 
    llvm::TrailingObjects<FunctionType, Type> {
    using TrailingObjects = llvm::TrailingObjects<FunctionType, Type>;
    friend TrailingObjects;
    public:
      using SizeTy = std::uint8_t;
      static constexpr auto maxParams = std::numeric_limits<SizeTy>::max();

      static FunctionType* get(ASTContext& ctxt, ArrayRef<Type> params,
        Type rtr);

      // Return true if this FunctionType's parameter types and return
      // type match the ones passed as parameters.
      bool isSame(ArrayRef<Type> params, Type rtr);

      Type getReturnType() const;
      ArrayRef<Type> getParamTypes() const;
      Type getParamType(std::size_t idx) const;
      SizeTy numParams() const;

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::FunctionType);
      }
      
    private:
      FunctionType(ArrayRef<Type> params, Type rtr);

      Type rtrType_;
      const SizeTy numParams_;
  };

  // ArrayType
  //    An array of a certain type (can be any type, 
  //    even another ArrayType)
  class ArrayType final : public TypeBase {
    public:
      // Returns the UNIQUE ArrayType instance for the given
      // type ty.
      static ArrayType* get(ASTContext& ctxt, Type ty);

      Type getElementType() const;

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::ArrayType);
      }

    private:
      ArrayType(Type elemTy);

      Type elementTy_= nullptr;
  };

  // LValueType
  //    C/C++-like LValue. e.g. This type is the one
  //    of a DeclRef when the declaration it refers to
  //    is not const.
  class LValueType final : public TypeBase {
    public:
      // Returns the UNIQUE LValueType instance for the given type "ty"
      static LValueType* get(ASTContext& ctxt, Type ty);

      Type getType() const;

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::LValueType);
      }

    private:
      LValueType(Type type);

      Type ty_ = nullptr;
  };

  // CellType
  class CellType final : public TypeBase {
    public:
      // Creates a new instance of the CellType class
      static CellType* create(ASTContext& ctxt);

      Type getSubst() const;
      bool hasSubst() const;
      void setSubst(Type type);
      void resetSubst();

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::CellType);
      }

    private:
      // Private because only called by ::create
      CellType();

      // Override the new operator to use the SemaAllocator in the
      // ASTContext to allocate CellTypes.
      void* operator new(std::size_t sz, ASTContext &ctxt, 
        std::uint8_t align = alignof(TypeBase));

      Type subst_ = nullptr;
  };

}
