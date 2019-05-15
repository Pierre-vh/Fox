//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Types.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file declares the TypeBase hierarchy.
//
// Note: a small (intentional) quirk of this hierarchy is the lack of a 
// 'MutType' for 'mut'. Mut is simply not represented in the type hierarchy 
// except in FunctionType (through FunctionTypeParam)
//----------------------------------------------------------------------------//

#pragma once

#include "ASTAligns.hpp"
#include "Type.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/Support/TrailingObjects.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/ArrayRef.h"
#include <string>
#include <cstdint>

namespace fox {
  /// Type Kinds
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

  /// TypeBase
  ///    Common base for types
  class alignas(TypeBaseAlignement) TypeBase {
    // Delete copy ctor/operator (can cause corruption with trailing objects)
    TypeBase(const TypeBase&) = delete;
    TypeBase& operator=(const TypeBase&) = delete;
    public:
      /// \returns the type's name in a user friendly form, 
      ///          e.g. "int", "string"
      std::string toString() const;

      /// \returns the type's name in a more "developer-friendly"
      ///          form, which provides more information.
      ///          e.g. "Array(int)" instead of [int]"
      std::string toDebugString() const;

      void dump() const;

      /// \returns the kind of type this is.
      TypeKind getKind() const;

      /// \returns for LValues, return the element's type of the LValue, else
      ///          returns this.
      Type getRValue();

      /// \returns for LValues, return the element's type of the LValue, else
      ///          returns this.
      const Type getRValue() const;

      /// \returns true if this Type tree contains a TypeVariable somewhere
      bool hasTypeVariable() const;

      /// \returns true if this Type tree contains an ErrorType somewhere
      bool hasErrorType() const;

      /// Note: all of the following functions are simply shorthand for
      /// type->getRValue()->is<...>(), this is done because most of these
      /// checks are frequently performed and this shorthand form
      /// helps with readability IMHO - Pierre

      /// \returns true if this type is the primitive 'string' type.
      /// Ignores LValues.
      bool isStringType() const;

      /// \returns true if this type is the primitive 'char' type.
      /// Ignores LValues.
      bool isCharType() const;

      /// \returns true if this type is the primitive 'double' type.
      /// Ignores LValues.
      bool isDoubleType() const;

      /// \returns true if this type is the primitive 'bool' type.
      /// Ignores LValues.
      bool isBoolType() const;

      /// \returns true if this type is the primitive 'int' type.
      /// Ignores LValues.
      bool isIntType() const;

      /// \returns true if this type is the primitive 'void' type.
      /// Ignores LValues
      bool isVoidType() const;

      /// \returns true if this type is either the primitive 'int' or 
      /// 'double' type. Ignores LValues.
      bool isNumericType() const;

      /// \returns true if this type is any primitive type. Ignores LValues.
      bool isPrimitiveType() const;

      /// \returns true if this type is either the primitive 'int', 
      /// 'double' or 'bool' type. Ignores LValues.
      bool isNumericOrBool() const;

      /// \returns true if this type is assignable (if it's an LValue)
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

      /// Represent a type's properties.
      class Properties {
        public:
          /// The integer type used to store the properties
          using ValueType = std::uint8_t;
          
          Properties(ValueType value = 0) : value_(value) {}

          /// Type properties
          enum Property : ValueType {
            /// This type contains a TypeVariableType somewhere
            /// in its hierarchy
            HasTypeVariable = 0x01,

            /// This type contains an ErrorType somewhere in it's
            /// hierarchy.
            HasErrorType = 0x02,

            last_property = HasErrorType
          };

          /// Intentionally implicit operator
          /// bool that checks that the value is non-zero.
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

      static_assert(Property::last_property < (1 << propsBits),
        "Too many properties in Properties::Property. "
        "Increase the number of bits used to store the properties' value "
        "in TypeBase. (Increase propsBits)");

      const TypeKind kind_ : 4;
      Properties::ValueType propsValue_ : propsBits;
  };

  /// BasicType
  ///    Common base for basic and primitive Types.
  class BasicType : public TypeBase {
    public:
      static bool classof(const TypeBase* type) {
        return ((type->getKind() >= TypeKind::First_BasicType) 
          && (type->getKind() <= TypeKind::Last_BasicType));
      }

    protected:
      BasicType(TypeKind tc);
  };

  /// PrimitiveType
  ///    Common base for primitive types
  class PrimitiveType : public BasicType {
    public:
      static bool classof(const TypeBase* type) {
        return ((type->getKind() >= TypeKind::First_PrimitiveType) 
          && (type->getKind() <= TypeKind::Last_PrimitiveType));
      }

    protected:
      PrimitiveType(TypeKind tc);
  };

  /// IntegerType
  ///   The primitive 'int' type.
  class IntegerType final : public PrimitiveType {
    public:
      static IntegerType* get(ASTContext& ctxt);

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::IntegerType);
      }
    private:
      IntegerType();
  };

  /// DoubleType
  ///   The primitive 'double' type.
  class DoubleType final : public PrimitiveType {
    public:
      static DoubleType* get(ASTContext& ctxt);

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::DoubleType);
      }
    private:
      DoubleType();
  };

  /// CharType
  ///   The primitive 'char' type.
  class CharType final : public PrimitiveType {
    public:
      static CharType* get(ASTContext& ctxt);

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::CharType);
      }
    private:
      CharType();
  };

  /// BoolType
  ///   The primitive 'bool' type.
  class BoolType final : public PrimitiveType {
    public:
      static BoolType* get(ASTContext& ctxt);

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::BoolType);
      }
    private:
      BoolType();
  };

  /// StringType
  ///   The primitive 'string' type.
  class StringType final : public PrimitiveType {
    public:
      static StringType* get(ASTContext& ctxt);

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::StringType);
      }
    private:
      StringType();
  };

  /// VoidType
  ///   The primitive 'void' type.
  class VoidType final : public PrimitiveType {
    public:
      static VoidType* get(ASTContext& ctxt);

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::VoidType);
      }
    private:
      VoidType();
  };

  /// ErrorType
  ///    A type used to represent that a expression's type
  ///    cannot be determined because of an error.
  class ErrorType final : public BasicType {
    public:
      /// \returns the unique ErrorType instance for \p ctxt
      static ErrorType* get(ASTContext& ctxt);

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::ErrorType);
      }

    private:
      ErrorType();
  };

  /// FunctionType
  ///    Represents the type of a function. 
  ///    Example: (int, int) -> int
  class FunctionType final : public TypeBase, 
      llvm::TrailingObjects<FunctionType, Type> {
    using TrailingObjects = 
      llvm::TrailingObjects<FunctionType, Type>;
    friend TrailingObjects;
    public:
      using SizeTy = std::size_t;
      static constexpr auto maxParams = std::numeric_limits<SizeTy>::max();

      static FunctionType* 
      get(ASTContext& ctxt, ArrayRef<Type> params, Type rtr);

      /// \returns true if this function's parameters and return type are
      /// all equal to \p params and \p rtr
      bool isSame(ArrayRef<Type> params, Type rtr);

      Type getReturnType() const;
      ArrayRef<Type> getParams() const;
      Type getParam(std::size_t idx) const;
      SizeTy numParams() const;

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::FunctionType);
      }
      
    private:
      FunctionType(ArrayRef<Type> params, Type rtr);

      static Properties 
      getPropertiesForFunc(ArrayRef<Type> params, Type rtr);

      Type rtrType_;
      const SizeTy numParams_;
  };

  /// ArrayType
  ///    An array of a certain type (can be any type, even another ArrayType)
  class ArrayType final : public TypeBase {
    public:
      /// \returns the unique ArrayType instance for the given type \p ty
      /// in the context \p ctxt
      static ArrayType* get(ASTContext& ctxt, Type ty);

      Type getElementType() const;

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::ArrayType);
      }

    private:
      ArrayType(Type elemTy);

      Type elementTy_= nullptr;
  };

  /// LValueType
  ///   A type that can appear on the LHS of an assignement.
  class LValueType final : public TypeBase {
    public:
      /// \returns the unique LValueType instance for the given type \p ty
      /// in the context \p ctxt
      static LValueType* get(ASTContext& ctxt, Type ty);

      Type getType() const;

      static bool classof(const TypeBase* type) {
        return (type->getKind() == TypeKind::LValueType);
      }

    private:
      LValueType(Type type);

      Type ty_ = nullptr;
  };

  /// TypeVariableType
  ///  A "Type Variable is introduced in places where type inference is needed.
  ///
  ///  This type is actually mutable, because, as an optimization,
  ///  the current substitution is stored in the TypeVariableType itself.
  ///  NOTE: The substitution is ignored in every TypeBase function, for
  ///  instance if the current subst is an ErrorType, we ->hasErrorType
  ///  will not return true.
  class TypeVariableType final : public TypeBase {
    public:
      /// Create a new TypeVariableType allocated in the context \p ctxt
      /// \param ctxt the ASTContext used to allocate the type variable
      /// \param number the number that this type variable should have
      static TypeVariableType* create(ASTContext& ctxt, std::uint16_t number);

      /// \returns the number assigned to this TypeVariable.
      std::uint16_t getNumber() const;

      /// \returns the current substitution
      Type getSubst() const;

      /// \returns true if this type has a substitution.
      bool hasSubst() const;

      /// If the current substitution is a TypeVariable too, returns
      /// its substitution. 
      /// 
      /// This recurses until we reach a nullptr or a substitution 
      /// that isn't a TypeVariable.
      Type getSubstRecursively() const;

      /// Assigns a substitution to this TypeVariable.
      /// This asserts that the current substitution is null (to
      /// avoid overwriting substitutions)
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
