////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Type.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Fox/AST/Type.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/AST/ASTContext.hpp"
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

const Type* Type::unwrapIfArray() const
{
	if (const ArrayType* tmp = dyn_cast<ArrayType>(this))
		return tmp->getElementType();
	return nullptr;
}

Type* Type::unwrapIfArray()
{
	if (ArrayType* tmp = dyn_cast<ArrayType>(this))
		return tmp->getElementType();
	return nullptr;
}

const Type* Type::unwrapIfLValue() const
{
	if (const LValueType* tmp = dyn_cast<LValueType>(this))
		return tmp->getType();
	return nullptr;
}

Type* Type::unwrapIfLValue()
{
	if (LValueType* tmp = dyn_cast<LValueType>(this))
		return tmp->getType();
	return nullptr;
}

const Type* Type::ignoreLValue() const
{
	auto* ptr = unwrapIfLValue();
	return ptr ? ptr : this;
}

Type* Type::ignoreLValue()
{
	auto* ptr = unwrapIfLValue();
	return ptr ? ptr : this;
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

PrimitiveType::Kind PrimitiveType::getPrimitiveKind() const
{
	return builtinKind_;
}

bool PrimitiveType::isString() const
{
	return builtinKind_ == Kind::StringTy;
}

bool PrimitiveType::isChar() const
{
	return builtinKind_ == Kind::CharTy;
}

bool PrimitiveType::isBool() const
{
	return builtinKind_ == Kind::BoolTy;
}

bool PrimitiveType::isInt() const
{
	return builtinKind_ == Kind::IntTy;
}

bool PrimitiveType::isFloat() const
{
	return builtinKind_ == Kind::FloatTy;
}

bool PrimitiveType::isVoid() const
{
	return builtinKind_ == Kind::VoidTy;
}

//-----------//
// ArrayType //
//-----------//

ArrayType::ArrayType(Type* elemTy):
	elementTy_(elemTy), BuiltinType(TypeKind::ArrayType)
{
	assert(elemTy && "The Array item type cannot be null!");
}

std::string ArrayType::getString() const
{
	return "Array(" + elementTy_->getString() + ")";
}

Type* ArrayType::getElementType()
{
	return elementTy_;
}

const Type* ArrayType::getElementType() const
{
	return elementTy_;
}

//------------//
// LValueType //
//------------//

LValueType::LValueType(Type* type):
	Type(TypeKind::LValueType), ty_(type)
{
	assert(type && "cannot be null");
}

std::string LValueType::getString() const
{
	// LValue types are represented by adding a prefix "@"
	return "@" + ty_->getString();
}

Type* LValueType::getType()
{
	return ty_;
}

const Type* LValueType::getType() const
{
	return ty_;
}

//----------//
// SemaType //
//----------//

SemaType::SemaType(Type* type):
	Type(TypeKind::SemaType), ty_(type)
{

}

std::string SemaType::getString() const
{
	return "SemaType(" + (ty_ ? ty_->getString() : "empty") + ")";
}

Type* SemaType::getSubstitution()
{
	return ty_;
}

const Type* SemaType::getSubstitution() const
{
	return ty_;
}

bool SemaType::hasSubstitution() const
{
	return (ty_ != nullptr);
}

void SemaType::setSubstitution(Type* subst)
{
	ty_ = subst;
}

void SemaType::reset()
{
	ty_ = nullptr;
}

ErrorType::ErrorType():
	Type(TypeKind::ErrorType)
{

}

std::string ErrorType::getString() const
{
	return "<error_type>";
}
