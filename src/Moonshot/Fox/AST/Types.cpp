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

/* Type */
bool Type::isPrimitive() const
{
	return false;
}

/* Primitive Types */
PrimitiveType::PrimitiveType(const Kind & kd) : builtinKind_(kd)
{

}

bool PrimitiveType::isBuiltin() const
{
	return true;
}

bool PrimitiveType::isPrimitive() const
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
			throw std::exception("Unknown Builtin Type");
	}
}

PrimitiveType::Kind PrimitiveType::getKind() const
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

void PrimitiveType::setBuiltinKind(const Kind & kd)
{
	builtinKind_ = kd;
}

/* Array type */
ArrayType::ArrayType(Type* ty) : itemTy_(std::move(ty))
{
	assert(ty && "The Array item type cannot be null!");
}

const Type* ArrayType::getItemTy() const
{
	return itemTy_;
}

bool ArrayType::isBuiltin() const
{
	return true;
}

std::string ArrayType::getString() const
{
	return itemTy_->getString() + "[]";
}

bool ArrayType::isItemTypePrimitive() const
{
	assert(itemTy_ && "The Array item type cannot be null!");
	return itemTy_->isPrimitive();
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

std::string QualType::getPrettyName() const
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

const Type* QualType::getType() const
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