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
	// Base abstract class for every Type node.
	// Types are immutable once created. As a result, every member function of type classes are marked "const".
	class Type
	{
		public:
			virtual ~Type() = 0;

			/* Should return true if the type is a PrimitiveType */
			virtual bool isPrimitiveType() const;

			/* Should return true if the type is a builtinType */
			virtual bool isBuiltinType() const;

			/* Should return true if the type is an ArrayType*/
			virtual bool isArrayType() const;

			/* Should return the type's name in a user friendly form, e.g. "int", "string" */
			virtual std::string getString() const = 0;
	};

	// Base abstract class for every builtin type.
	class BuiltinType : public Type
	{
		public:	
			virtual ~BuiltinType() = 0;

			virtual bool isBuiltinType() const;
	};

	// PrimitiveType
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

			virtual bool isPrimitiveType() const override;
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

	// Array types
	class ArrayType : public BuiltinType
	{
		public:
			ArrayType(const Type* itemsTy);

			virtual bool isArrayType() const override;
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
	class QualType
	{
		public:
			QualType() = default;
			QualType(const Type* ty, const bool& isConstant = false,const bool &isReference = false);

			// Const
			bool isConstant() const;
			void setConstAttribute(const bool& constattr);
			
			// Ref
			bool isAReference() const;
			void setIsReference(const bool& refattr);

			// Returns a name for the type, with the attributes.
			// e.g. "const &int"
			std::string getString() const;

			// Returns the Type pointer (ty_)
			const Type* getType() const;
			void setType(const Type* ty);

			// Checks if this QualType is valid (ty_ != nullptr)
			bool isValid() const;
			operator bool() const;
		private:
			const Type* ty_ = nullptr;
			bool isConst_ : 1;
			bool isRef_ : 1;
	};
}