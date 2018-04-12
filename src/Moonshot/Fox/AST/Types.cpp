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
#include <sstream>
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
		case Kind::VoidTy:
			return "void";
		default:
			throw std::exception("Unknown Builtin Type");
	}
}

bool BuiltinType::isBasic() const
{
	// Only basic if not void.
	return !(builtinKind_ == Kind::VoidTy);
}

BuiltinType::Kind BuiltinType::getKind() const
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

void BuiltinType::setBuiltinKind(const Kind & kd)
{
	builtinKind_ = kd;
}

/* Array type */
ArrayType::ArrayType(TypePtr ty) : itemTy_(std::move(ty))
{
	assert(ty && "The Array item type cannot be null!");
}

TypePtr ArrayType::getItemTy()
{
	return itemTy_;
}

void ArrayType::setItemType(TypePtr ptr)
{
	assert(ptr && "The Array item type cannot be null!");
	itemTy_ = std::move(ptr);
}

bool ArrayType::isBuiltin() const
{
	return true;
}

std::string ArrayType::getPrettyTypeName() const
{
	return itemTy_->getPrettyTypeName() + "[]";
}

bool ArrayType::isBasic() const
{
	return false;
}

bool ArrayType::isItemTypeBasic() const
{
	assert(itemTy_ && "The Array item type cannot be null!");
	return itemTy_->isBasic();
}

bool ArrayType::isItemTypeBuiltin() const
{
	assert(itemTy_ && "The Array item type cannot be null!");
	return itemTy_->isBuiltin();
}

/* QualType */
QualType::QualType() : ty_(nullptr),isConst_(false), isRef_(false)
{

}

QualType::QualType(TypePtr ty, const bool & isConstant, const bool &isReference) :
	ty_(std::move(ty)), isConst_(isConstant), isRef_(isReference)
{
	assert(ty && "Type cannot be null");
}

bool QualType::isConstant() const
{
	return isConst_;
}

void QualType::setConstAttribute(const bool & constattr)
{
	isConst_ = constattr;
}

bool QualType::isAReference() const
{
	return isRef_;
}

void QualType::setIsReference(const bool & refattr)
{
	isRef_ = refattr;
}

std::string QualType::getPrettyName() const
{
	std::stringstream out;
	if (isConst_)
		out << "const ";
	if (isRef_)
		out << "&";
	assert(ty_ && "Type is null?");
	out << ty_->getPrettyTypeName();
	return out.str();
}

TypePtr QualType::getNonQualType()
{
	assert(ty_ && "Type is null?");
	return ty_;
}

void QualType::setType(TypePtr ty)
{
	assert(ty && "Type is null?");
	ty_ = std::move(ty);
}

bool QualType::isValid() const
{
	return (bool)ty_;
}

QualType::operator bool() const
{
	return isValid();
}