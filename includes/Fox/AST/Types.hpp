//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Types.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the TypeBase hierarchy.
//----------------------------------------------------------------------------//

#pragma once

#include "ASTAligns.hpp"
#include "Type.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/Support/TrailingObjects.h"
#include "llvm/ADT/ArrayRef.h"
#include <string>
#include <cstdint>

namespace fox {
  // Kinds of Types
  enum class TypeKind : std::uint8_t {
    #define TYPE(ID,PARENT) ID,
    #define TYPE_RANGE(ID,FIRST,LAST) First_##ID = FIRST, Last_##ID = LAST,
    #define LAST_TYPE(ID) Last_Type
    #include "TypeNodes.def"
  };

  inline constexpr auto toInt(TypeKind kind) {
    return static_cast<std::underlying_type<TypeKind>::type>(kind);
  }

  // Forward Declarations
  class ASTContext;

  // TypeBase
  //    Common base for types
  //
  // Usually, you should never use raw TypeBase* pointers unless you have
  // a valid reason. Always use the Type wrapper. (see Type.hpp)
  class alignas(TypeBaseAlignement) TypeBase {
    // Delete copy ctor/operator (can cause corruption with trailing objects)
    TypeBase(const TypeBase&) = delete;
    TypeBase& operator=(const TypeBase&) = delete;
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

      // If this type is an LValue, returns it's element type, else
      // returns this.
      Type getRValue();

      // Returns true if this Type contains a TypeVariable somewhere
      // in it's hierarchy.
      bool hasTypeVariable() const;

      // Returns true if this Type contains a ErrorType somewhere
      // in it's hierarchy.
      bool hasErrorType() const;

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
      // Type properties
      //-------------------------//

      // This class represent a type's properties.
      class Properties {
        public:
          // The integer type used internally to store
          // the properties.
          using ValueType = std::uint8_t;
          
          Properties(ValueType value = 0) : value_(value) {}

          // The properties enum
          enum Property : ValueType {
            // This type contains a TypeVariableType somewhere
            // in it's hierarchy
            HasTypeVariable = 0x01,

            // This type contains an ErrorType somewhere in it's
            // hierarchy.
            HasErrorType = 0x02,

            LastProperty = HasErrorType
          };

          // Intentionally implicit operator
          // bool which simply checks that the value is non-zero.
          /*implicit*/ operator bool() const {
            return value_ != 0;
          }

          Properties operator&(Properties prop) {
            return Properties(value_ & prop.value_);
          }

          Properties& operator&=(Properties prop) {
            value_ &= prop.value_;
            return *this;
          }

          Properties operator|(Properties prop) {
            return Properties(value_ | prop.value_);
          }

          Properties& operator|=(Properties prop) {
            value_ |= prop.value_;
            return *this;
          }

          Properties operator&(Property prop) {
            return Properties(value_ & prop);
          }

          Properties& operator&=(Property prop) {
            value_ &= prop;
            return *this;
          }

          Properties operator|(Property prop) {
            return Properties(value_ | prop);
          }

          Properties& operator|=(Property prop) {
            value_ |= prop;
            return *this;
          }

          ValueType getValue() const {
            return value_;
          }

        private:
          ValueType value_ = 0;
      };

      using Property = Properties::Property;

      Properties getProperties() const;

      // Prohibit the use of builtin placement new & delete
      void *operator new(std::size_t) noexcept = delete;
      void operator delete(void *) noexcept = delete;

    protected:
      TypeBase(TypeKind tc);

      // Only allow allocation through the ASTContext
      void* operator new(std::size_t sz, ASTContext &ctxt, 
      std::uint8_t align = alignof(TypeBase));

      // And through placement new
      void* operator new(std::size_t, void* buff);

      // Set the properties for this TypeBase. This should
      // only be called in the constructor, and should
      // only be called once. An assertion guarantees it.
      void setProperties(Properties props);

    private:
      static constexpr unsigned propsBits = 4;

      static_assert(toInt(TypeKind::Last_Type) < (1 << 4),
        "Too many types in TypeKind. Increase the number of bits used"
        " to store the TypeKind in TypeBase");

      static_assert(Property::LastProperty < (1 << propsBits),
        "Too many properties in Properties::Property. "
        "Increase the number of bits used to store the properties' value "
        "in TypeBase. (Increase propsBits)");

      const TypeKind kind_ : 4;
      Properties::ValueType propsValue_ : propsBits;
  };

  // BasicType
  //    Common base for "Basic" Types.
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

      static FunctionType* 
      get(ASTContext& ctxt, ArrayRef<Type> params, Type rtr);

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

      static Properties getPropertiesForFunc(ArrayRef<Type> params, Type rtr);

      Type rtrType_;
      const SizeTy numParams_;
  };

  // ArrayType
  //    An array of a certain type (can be any type, 
  //    even another ArrayType)
  class ArrayType final : public TypeBase {
    public:
      // Returns the unique ArrayType instance for the given
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
      // Returns the unique LValueType instance for the given type "ty"
      static LValueType* get(ASTContext& ctxt, Type ty);

      Type getType() const;

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::LValueType);
      }

    private:
      LValueType(Type type);

      Type ty_ = nullptr;
  };

  // TypeVariableType
  //  A "Type Variable", e.g. "T0", introduced in places
  //  where type inference is required.
  //
  //  This type is actually mutable, because, as an optimization,
  //  the current substitution is stored in the TypeVariableType, this avoids
  //  map lookups when we want to retrieve the current substitution.
  //
  //  However, the current substitution should be ignored most of the time,
  //  except in semantic analysis.
  //  For instance, when printing this type for the user, don't show the 
  //  substitution, just show "any".
  class TypeVariableType final : public TypeBase {
    public:
      static TypeVariableType* create(ASTContext& ctxt, std::uint16_t number);

      // Returns the number assigned to this TypeVariable.
      std::uint16_t getNumber() const;

      // Returns the current substitution
      Type getSubst() const;

      // Returns true if this type has a substitution.
      bool hasSubst() const;

      // If the current substitution is a TypeVariable too, returns
      // it's substitution. 
      // 
      // This recurses until we reach a nullptr or a substitution 
      // that isn't a TypeVariable.
      Type getSubstRecursively() const;

      // Assigns a substitution to this TypeVariable.
      // This asserts that the current substitution is null.
      void assignSubst(Type type);

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::TypeVariableType);
      }

    private:
      TypeVariableType(std::uint16_t number);

      Type subst_;
      std::uint16_t number_ = 0;
  };
}
