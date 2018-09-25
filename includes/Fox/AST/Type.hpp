////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Type.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the Type AST nodes.
////------------------------------------------------------////

#pragma once

#include <string>
#include <cstdint>

namespace fox
{
	// Kinds of Types
	enum class TypeKind : std::uint8_t
	{
		#define TYPE(ID,PARENT) ID,
		#define TYPE_RANGE(ID,FIRST,LAST) First_##ID = FIRST, Last_##ID = LAST,
		#include "TypeNodes.def"
	};

	// Forward Declarations
	class ASTContext;

	// Type
	//		Common base for types
	class Type
	{
		public:
			/* Should return the type's name in a user friendly form, e.g. "int", "string" */
			virtual std::string getString() const = 0;

			TypeKind getKind() const;

			// Returns the element type if this is an ArrayType, or nullptr.
			const Type* unwrapIfArray() const;
			Type* unwrapIfArray();

			// Returns the element type if this is an LValueType, or nullptr.
			const Type* unwrapIfLValue() const;
			Type* unwrapIfLValue();

			// If this type is an LValue, returns it's element type, else
			// returns this.
			const Type* ignoreLValue() const;
			Type* ignoreLValue();

		protected:
			Type(TypeKind tc);

			friend class ASTContext;

			// Prohibit the use of builtin placement new & delete
			void *operator new(std::size_t) throw() = delete;
			void operator delete(void *) throw() = delete;
			void* operator new(std::size_t, void*) = delete;

			// Only allow allocation through the ASTContext
			// This operator is "protected" so only the ASTContext can create types.
			void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(Type));

			// Companion operator delete to silence C4291 on MSVC
			void operator delete(void*, ASTContext&, std::uint8_t) {}

		private:
			const TypeKind kind_;
	};

	// BuiltinType
	//		Common base for Built-in types
	class BuiltinType : public Type
	{
		public:
			static bool classof(const Type* type)
			{
				return ((type->getKind() >= TypeKind::First_BuiltinType) 
					&& (type->getKind() <= TypeKind::Last_BuiltinType));
			}

		protected:
			BuiltinType(TypeKind tc);
	};

	// PrimitiveType 
	//		A primitive type (void/int/float/char/bool/string)
	class PrimitiveType : public BuiltinType
	{
		public:
			enum class Kind : std::uint8_t
			{
				VoidTy,
				IntTy,
				FloatTy,
				CharTy,
				StringTy,
				BoolTy
			};

			PrimitiveType(Kind kd);

			virtual std::string getString() const override;

			Kind getPrimitiveKind() const;

			// Returns true iff builtinKind_ == IntTy, FloatTy or BoolTy
			bool isIntegral() const;

			bool isString() const;
			bool isChar() const;
			bool isFloat() const;
			bool isBool() const;
			bool isInt() const;
			bool isVoid() const;

			static bool classof(const Type* type)
			{
				return (type->getKind() == TypeKind::PrimitiveType);
			}

		private:
			const Kind builtinKind_;
	};

	// ArrayType
	//		An array of a certain type (can be any type, 
	//		even another ArrayType)
	class ArrayType : public BuiltinType
	{
		public:
			ArrayType(Type* elemTy);

			virtual std::string getString() const override;

			Type* getElementType();
			const Type* getElementType() const;

			static bool classof(const Type* type)
			{
				return (type->getKind() == TypeKind::ArrayType);
			}

		private:
			Type* elementTy_= nullptr;
	};

	// LValueType
	//		C/C++-like LValue. e.g. This type is the one
	//		of a DeclRef when the declaration it refers to
	//		is not const.
	class LValueType : public Type
	{
		public:
			LValueType(Type* type);

			virtual std::string getString() const override;

			Type* getType();
			const Type* getType() const;

			static bool classof(const Type* type)
			{
				return (type->getKind() == TypeKind::LValueType);
			}

		private:
			Type* ty_ = nullptr;
	};

	// SemaType
	//		Type used in semantic analysis to perform
	//		basic, local type inference. This is, like LValueType & ArrayType,
	//		just a wrapper around a Type*, however the pointer may be null if no
	//		substitution was chosen.
	class SemaType : public Type
	{
		public:
			SemaType(Type* type = nullptr);

			virtual std::string getString() const override;

			Type* getSubstitution();
			const Type* getSubstitution() const;

			// Returns true if the type has a substitution
			// (type isn't null)
			bool hasSubstitution() const;

			void setSubstitution(Type* subst);

			// Sets the type to nullptr
			void reset();

			static bool classof(const Type* type)
			{
				return (type->getKind() == TypeKind::SemaType);
			}
		private:
			Type * ty_ = nullptr;
	};

	// ErrorType
	//		A type used to represent that a expression's type
	//		cannot be determined because of an error.
	class ErrorType : public Type
	{
		public:
			ErrorType();

			virtual std::string getString() const override;

			static bool classof(const Type* type)
			{
				return (type->getKind() == TypeKind::ErrorType);
			}
	};
}