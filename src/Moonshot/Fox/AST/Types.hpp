////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Types.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the Type AST nodes.
////------------------------------------------------------////

#pragma once

#include <string>
#include <memory>

namespace Moonshot
{
	// Base class for every Type node.
	class Type
	{
		public:
			/* Should return true if the type is a builtin type (= compiler builtin, not user defined)*/
			virtual bool isBuiltin() const = 0;

			/* Should return true if the type is a primitive type */
			/* Primitives types are the types contained in the PrimitiveType class */
			virtual bool isPrimitive() const;

			/* Should return the type's name in a user friendly form, e.g. "int", "string" */
			virtual std::string getString() const = 0;
	};

	//	Builtin Types
	//	Builtin types are:
	//		One of the following : int, float, char, string, bool
	//		Basic 
	//		Primitive
	//		Builtin
	class PrimitiveType : public Type
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

			// Ctor
			PrimitiveType() = default;
			PrimitiveType(const Kind& kd);

			// Methods inherited from Type
			virtual bool isBuiltin() const override;
			virtual bool isPrimitive() const override;
			virtual std::string getString() const override;

			// return the kind of this builtin
			Kind getKind() const;

			// Returns true if this type is considered "Arithmetic". 
			// Returns true iff builtinKind_ == IntTy, FloatTy or BoolTy
			bool isArithmetic() const;

			// Returns true if a concatenation operator can be applied to this type
			// Returns true iff builtinKind_ == StringTy or CharTy
			bool isConcatenable() const;

			// Returns true iff builtinKind_ == Kind::VoidTy
			bool isVoid() const;
		private:
			friend class ASTContext;

			void setBuiltinKind(const Kind& kd);

			Kind builtinKind_;
	};

	// Array types
	// Arrays are:
	//		Builtin
	class ArrayType : public Type
	{
		public:
			ArrayType(Type* itemsTy);

			virtual bool isBuiltin() const override;
			virtual std::string getString() const override;

			const Type* getItemTy() const;
			bool isItemTypePrimitive() const;
			bool isItemTypeBuiltin() const;
		private:
			Type* itemTy_= nullptr;
	};

	// QualType is a class that groups a pointer to a Type as well as qualifiers 
	// Qualifiers include : const (true/false) and reference (true/false)
	class QualType
	{
		public:
			QualType();
			QualType(Type* ty, const bool& isConstant,const bool &isReference);

			// Const
			bool isConstant() const;
			void setConstAttribute(const bool& constattr);
			
			// Ref
			bool isAReference() const;
			void setIsReference(const bool& refattr);

			// Returns a "pretty" type name for this type, useful
			// for outputting the type to the user in a dump or in a diag.
			std::string getPrettyName() const;

			// Returns the Type pointer (ty_)
			const Type* getType() const;
			void setType(Type* ty);

			// Checks if this QualType is valid (ty_ != nullptr)
			bool isValid() const;
			operator bool() const;
		private:
			Type* ty_ = nullptr;
			bool isConst_ : 1;
			bool isRef_ : 1;
	};
}