////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Type.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the Type AST nodes.
//	
// TODO: I think that in the future a "ErrorType" could be really handy for parser error recovery
// and TypeChecking errors handling.
//
// At the time of writing this, work on the Semantic Analyzer has not
// started yet. Thus, most classes here will be changed a lot
// in the future. Also, the types are all trivial (builtin types & array types)
// which means that they don't have any complicated work to do, 
//
////------------------------------------------------------////

#pragma once

#include <string>

namespace fox
{
	// The TypeKind enum
	enum class TypeKind : char
	{
		#define TYPE(ID,PARENT) ID,
		#include "TypeNodes.def"
	};

	// Base abstract class for every Type node.
	// Types are immutable once created.
	// As a result, most member function of type classes are marked "const", with
	// a few exceptions.
	class Type
	{
		public:
			bool isPrimitiveType() const;
			bool isBuiltinType() const;
			bool isArrayType() const;

			/* Should return the type's name in a user friendly form, e.g. "int", "string" */
			virtual std::string getString() const = 0;

			TypeKind getKind() const;
		protected:
			Type(const TypeKind& tc, const bool& isPrimitive, const bool& isBuiltin, const bool& isArray);
		private:
			TypeKind kind_;

			// Bitfield //
			const bool isPrimitive_ : 1;
			const bool isBuiltin_ : 1;
			const bool isArray_ : 1;
			// 5 Bits left //
	};

	// Base abstract class for every builtin type.
	// Built-in -> Types that are built-in the language, that
	// are not user created.
	class BuiltinType : public Type
	{
		protected:
			BuiltinType(const TypeKind& tc,const bool& isPrimitive, const bool& isArray);
	};

	// PrimitiveType (builtin,primitive)
	//		int, float, char, string, bool, void
	class PrimitiveType : public BuiltinType
	{
		public:
			enum class Kind
			{
				VoidTy,
				IntTy,
				FloatTy,
				CharTy,
				StringTy,
				BoolTy
			};

			PrimitiveType(const Kind& kd);

			virtual std::string getString() const override;

			Kind getBuiltinKind() const;

			// Returns true iff builtinKind_ == IntTy, FloatTy or BoolTy
			bool isArithmetic() const;

			// Returns true iff builtinKind_ == StringTy or CharTy
			bool isConcatenable() const;

			// Returns true iff builtinKind_ == Kind::VoidTy
			bool isVoid() const;
		private:
			Kind builtinKind_;
	};

	// Array types (builtin)
	class ArrayType : public BuiltinType
	{
		public:
			ArrayType(const Type* itemsTy);

			virtual std::string getString() const override;

			const Type* getItemTy() const;

			bool isItemTypePrimitive() const;
			bool isItemTypeBuiltin() const;
			bool isItemTypeArray() const; 

		private:
			const Type* itemTy_= nullptr;
	};

	// QualType is a class that groups a pointer to a Type as well as qualifiers 
	// Qualifiers include : const (true/false) and reference (true/false)
	// Note that this class is not a part of the Type hierarchy.
	class QualType
	{
		public:
			QualType() = default;
			QualType(Type* ty, const bool& isConstant = false,const bool &isReference = false);

			// Const
			bool isConstant() const;
			void setIsConst(const bool& constattr);
			
			// Ref
			bool isReference() const;
			void setIsReference(const bool& refattr);

			// Returns a name for the type, with the attributes.
			// e.g. "const &int"
			std::string getString() const;

			// Returns the Type pointer (ty_)
			Type* getType();
			const Type* getType() const;
			void setType(Type* ty);

			// Checks if this QualType is valid (ty_ != nullptr)
			bool isValid() const;
			explicit operator bool() const;
		private:
			Type* ty_ = nullptr;

			// Bitfield //
			bool isConst_ : 1;
			bool isRef_ : 1;
			// 6 Bits left //
	};
}