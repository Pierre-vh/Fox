////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Types.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Type.hpp"
#include "Fox/Common/Utils.hpp"

#include <sstream>

using namespace fox;

/* Type */
Type::Type(const TypeKind & tc) : kind_(tc)
{

}

Type::~Type()
{

}

bool Type::isPrimitiveType() const
{
	return false;
}

bool Type::isBuiltinType() const
{
	return false;
}

bool Type::isArrayType() const
{
	return false;
}

TypeKind Type::getKind() const
{
	return kind_;
}

/* BuiltinType */
BuiltinType::BuiltinType(const TypeKind & tc) : Type(tc)
{

}

BuiltinType::~BuiltinType()
{

}

bool BuiltinType::isBuiltinType() const
{
	return true;
}

/* Primitive Types */
PrimitiveType::PrimitiveType(const Kind & kd) : builtinKind_(kd), BuiltinType(TypeKind::PrimitiveType)
{

}

bool PrimitiveType::isPrimitiveType() const
{
	return true;
}

std::string PrimitiveType::getString() const
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
			fox_unreachable(); // Unknown builtin kind
			return "";
	}
}

PrimitiveType::Kind PrimitiveType::getBuiltinKind() const
{
	return builtinKind_;
}

bool PrimitiveType::isArithmetic() const
{
	return (builtinKind_ == Kind::IntTy) || (builtinKind_ == Kind::BoolTy) || (builtinKind_ == Kind::FloatTy);
}

bool PrimitiveType::isConcatenable() const
{
	return (builtinKind_ == Kind::CharTy) || (builtinKind_ == Kind::StringTy);
}

bool PrimitiveType::isVoid() const
{
	return (builtinKind_ == Kind::VoidTy);
}

/* Array type */
ArrayType::ArrayType(Type* ty) : itemTy_(ty), BuiltinType(TypeKind::ArrayType)
{
	assert(ty && "The Array item type cannot be null!");
}

bool ArrayType::isArrayType() const
{
	return true;
}

std::string ArrayType::getString() const
{
	return itemTy_->getString() + "[]";
}

Type* ArrayType::getItemTy()
{
	return itemTy_;
}

bool ArrayType::isItemTypePrimitive() const
{
	assert(itemTy_ && "The Array item type cannot be null!");
	return itemTy_->isPrimitiveType();
}

bool ArrayType::isItemTypeBuiltin() const
{
	assert(itemTy_ && "The Array item type cannot be null!");
	return itemTy_->isBuiltinType();
}

bool ArrayType::isItemTypeArray() const
{
	assert(itemTy_ && "The Array item type cannot be null!");
	return itemTy_->isArrayType();
}

/* QualType */
QualType::QualType(Type* ty, const bool & isConstant, const bool &isReference) :
	ty_(ty), isConst_(isConstant), isRef_(isReference)
{
	assert(ty_ && "The Type cannot be null!");
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

std::string QualType::getString() const
{
	std::stringstream out;
	if (isConst_)
		out << "const ";
	if (isRef_)
		out << "&";

	assert(ty_ && "Can't call this on a null type!");
	out << ty_->getString();
	return out.str();
}

Type* QualType::getType()
{
	return ty_;
}

void QualType::setType(Type* ty)
{
	ty_ = ty;
}

bool QualType::isValid() const
{
	return (bool)ty_;
}

QualType::operator bool() const
{
	return isValid();
}