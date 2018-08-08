////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Type.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Type.hpp"
#include "Fox/Common/Errors.hpp"
#include "ASTContext.hpp"
#include <sstream>

using namespace fox;

//------//
// Type //
//------//

Type::Type(TypeKind tc) : kind_(tc)
{

}

TypeKind Type::getKind() const
{
	return kind_;
}

void* Type::operator new(size_t sz, ASTContext& ctxt, std::uint8_t align)
{
	return ctxt.getAllocator().allocate(sz, align);
}

//-------------//
// BuiltinType //
//-------------//

BuiltinType::BuiltinType(TypeKind tc) : Type(tc)
{

}

//---------------//
// PrimitiveType //
//---------------//

PrimitiveType::PrimitiveType(Kind kd)
	: builtinKind_(kd), BuiltinType(TypeKind::PrimitiveType)
{

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
			fox_unreachable("Unknown builtin kind");
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

//-----------//
// ArrayType //
//-----------//

ArrayType::ArrayType(Type* ty) : itemTy_(ty), BuiltinType(TypeKind::ArrayType)
{
	assert(ty && "The Array item type cannot be null!");
}

std::string ArrayType::getString() const
{
	return itemTy_->getString() + "[]";
}

Type* ArrayType::getItemTy() const
{
	return itemTy_;
}

//----------//
// QualType //
//----------//

QualType::QualType() 
	: ty_(nullptr), isConst_(false), isRef_(false)
{

}

QualType::QualType(Type* ty, bool isConstant, bool isReference)
	: ty_(ty), isConst_(isConstant), isRef_(isReference)
{
	assert(ty_ && "The Type cannot be null!");
}

bool QualType::isConstant() const
{
	return isConst_;
}

void QualType::setIsConst(bool constattr)
{
	isConst_ = constattr;
}

bool QualType::isReference() const
{
	return isRef_;
}

void QualType::setIsReference(bool refattr)
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

Type* QualType::getType() const
{
	return ty_;
}

void QualType::setType(Type* ty)
{
	assert(ty && "The type cannot be null!");
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