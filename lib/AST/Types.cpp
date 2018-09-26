////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Types.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Fox/AST/Types.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/AST/ASTContext.hpp"
#include <sstream>

using namespace fox;

//------//
// TypeBase //
//------//

TypeBase::TypeBase(TypeKind tc) : kind_(tc)
{

}

TypeKind TypeBase::getKind() const
{
	return kind_;
}

const TypeBase* TypeBase::unwrapIfArray() const
{
	if (const ArrayType* tmp = dyn_cast<ArrayType>(this))
		return tmp->getElementType();
	return nullptr;
}

TypeBase* TypeBase::unwrapIfArray()
{
	if (ArrayType* tmp = dyn_cast<ArrayType>(this))
		return tmp->getElementType();
	return nullptr;
}

const TypeBase* TypeBase::unwrapIfLValue() const
{
	if (const LValueType* tmp = dyn_cast<LValueType>(this))
		return tmp->getType();
	return nullptr;
}

TypeBase* TypeBase::unwrapIfLValue()
{
	if (LValueType* tmp = dyn_cast<LValueType>(this))
		return tmp->getType();
	return nullptr;
}

const TypeBase* TypeBase::ignoreLValue() const
{
	auto* ptr = unwrapIfLValue();
	return ptr ? ptr : this;
}

TypeBase* TypeBase::ignoreLValue()
{
	auto* ptr = unwrapIfLValue();
	return ptr ? ptr : this;
}

void* TypeBase::operator new(size_t sz, ASTContext& ctxt, std::uint8_t align)
{
	return ctxt.getAllocator().allocate(sz, align);
}

//-------------//
// BuiltinType //
//-------------//

BuiltinType::BuiltinType(TypeKind tc) : TypeBase(tc)
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

ArrayType::ArrayType(TypeBase* elemTy):
	elementTy_(elemTy), BuiltinType(TypeKind::ArrayType)
{
	assert(elemTy && "The Array item type cannot be null!");
}

std::string ArrayType::getString() const
{
	return "Array(" + elementTy_->getString() + ")";
}

TypeBase* ArrayType::getElementType()
{
	return elementTy_;
}

const TypeBase* ArrayType::getElementType() const
{
	return elementTy_;
}

//------------//
// LValueType //
//------------//

LValueType::LValueType(TypeBase* type):
	TypeBase(TypeKind::LValueType), ty_(type)
{
	assert(type && "cannot be null");
}

std::string LValueType::getString() const
{
	// LValue types are represented by adding a prefix "@"
	return "@" + ty_->getString();
}

TypeBase* LValueType::getType()
{
	return ty_;
}

const TypeBase* LValueType::getType() const
{
	return ty_;
}

//----------//
// SemaType //
//----------//

SemaType::SemaType(TypeBase* type):
	TypeBase(TypeKind::SemaType), ty_(type)
{

}

std::string SemaType::getString() const
{
	return "SemaType(" + (ty_ ? ty_->getString() : "empty") + ")";
}

TypeBase* SemaType::getSubstitution()
{
	return ty_;
}

const TypeBase* SemaType::getSubstitution() const
{
	return ty_;
}

bool SemaType::hasSubstitution() const
{
	return (ty_ != nullptr);
}

void SemaType::setSubstitution(TypeBase* subst)
{
	ty_ = subst;
}

void SemaType::reset()
{
	ty_ = nullptr;
}

ErrorType::ErrorType():
	TypeBase(TypeKind::ErrorType)
{

}

std::string ErrorType::getString() const
{
	return "<error_type>";
}
