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
namespace Moonshot
{
	// Base class for every Type node.
	class IType
	{
		public:
			/* Should return true if the type is a builtin type*/
			virtual bool isBuiltin() const = 0;

			/* Should return true if this type is basic. */
			/* Basic = Builtin Types that are not arrays.*/
			virtual bool isBasic() const = 0;

			/* Should return the type's name in a user friendly form, e.g. "int", "string" */
			virtual std::string getPrettyTypeName() const = 0;
	};

	//	Builtin Types
	//	Builtin types are:
	//		One of the following : int, float, char, string, bool
	//		Basic
	//		Builtin
	class BuiltinType : public IType
	{
		public:
			enum class Kind
			{
				IntTy,
				FloatTy,
				CharTy,
				StringTy,
				BoolTy
			};

			// Ctor
			BuiltinType(const Kind& kd);

			// Methods inherited from IType
			virtual bool isBuiltin() const override;
			virtual std::string getPrettyTypeName() const override;
			virtual bool isBasic() const override;

			// return the kind of this builtin
			Kind getKind() const;

			// Returns true if this type is considered "Arithmetic". 
			// Returns true iff builtinKind_ == IntTy, FloatTy or BoolTy
			bool isArithmetic() const;

			// Returns true if a concatenation operator can be applied to this type
			// Returns true iff builtinKind_ == StringTy or CharTy
			bool isConcatenable() const;
		private:
			Kind builtinKind_;
	};

	// Array types
	// Arrays are:
	//		Builtin
	//		Non-Basic
	class ArrayType : public IType
	{
		public:
			ArrayType(IType *ty);

			virtual bool isBuiltin() const override;
			virtual std::string getPrettyTypeName() const override;
			virtual bool isBasic() const override;

			bool isItemTypeBasic() const;
			bool isItemTypeBuiltin() const;
		private:
			IType * itemsTy_ = nullptr;
	};
}