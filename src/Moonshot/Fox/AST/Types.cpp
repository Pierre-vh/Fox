////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Types.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Types.hpp"
#include <cassert>
#include <exception>

using namespace Moonshot;

/* Builtin Types */
BuiltinType::BuiltinType(const Kind & kd) : builtinKind_(kd)
{

}

bool BuiltinType::isBuiltin() const
{
	return true;
}

std::string BuiltinType::getPrettyTypeName() const
{
	switch (builtinKind_)
	{
		case Kind::IntTy:
			return "int";
		case Kind::BoolTy:
			return "bool";
		case Kind::CharTy:
			return "char";
		case Kind::FloatTy:
			return "float";
		case Kind::StringTy:
			return "string";
		default:
			throw std::exception("Unknown Builtin Type");
	}
}

bool BuiltinType::isBasic() const
{
	return true;
}

Kind BuiltinType::getKind() const
{
	return builtinKind_;
}

bool BuiltinType::isArithmetic() const
{
	return (builtinKind_ == Kind::IntTy) || (builtinKind_ == Kind::BoolTy) || (builtinKind_ == Kind::FloatTy);
}

bool BuiltinType::isConcatenable() const
{
	return (builtinKind_ == Kind::CharTy) || (builtinKind_ == Kind::StringTy);
}

/* Array type */
ArrayType::ArrayType(IType * ty) : itemsTy_(ty)
{
	assert(ty && "The Array item type cannot be null!");
}

bool ArrayType::isBuiltin() const
{
	return true;
}

std::string ArrayType::getPrettyTypeName() const
{
	return itemsTy_->getTypeName() + "[]";
}

bool ArrayType::isBasic() const
{
	return false;
}

bool ArrayType::isItemTypeBasic() const
{
	assert(itemsTy_ && "The Array item type cannot be null!");
	return itemsTy_->isBasic();
}

bool ArrayType::isItemTypeBuiltin() const
{
	assert(itemsTy_ && "The Array item type cannot be null!");
	return itemsTy_->isBuiltin();
}
