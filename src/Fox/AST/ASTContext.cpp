////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTContext.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTContext.hpp"

using namespace fox;

ASTContext::ASTContext()
{
	// Init builtin types
	initBuiltinTypes();
}

UnitDecl* ASTContext::getMainUnit()
{
	return mainUnit_;
}

void ASTContext::addUnit(UnitDecl* unit, bool isMainUnit)
{
	units_.push_back(unit);

	if (isMainUnit)
		mainUnit_ = unit;
}

PrimitiveType* ASTContext::getIntType()
{
	return intTy_;
}

PrimitiveType* ASTContext::getFloatType()
{
	return floatTy_;
}

PrimitiveType* ASTContext::getCharType()
{
	return charTy_;
}

PrimitiveType* ASTContext::getBoolType()
{
	return boolTy_;
}

PrimitiveType* ASTContext::getStringType()
{
	return stringTy_;
}

PrimitiveType* ASTContext::getVoidType()
{
	return voidTy_;
}

void ASTContext::initBuiltinTypes()
{
	if (!voidTy_)
		voidTy_ = new(*this) PrimitiveType(PrimitiveType::Kind::VoidTy);

	if (!boolTy_)
		boolTy_ = new(*this) PrimitiveType(PrimitiveType::Kind::BoolTy);

	if (!stringTy_)
		stringTy_ = new(*this) PrimitiveType(PrimitiveType::Kind::StringTy);

	if (!charTy_)
		charTy_ = new(*this) PrimitiveType(PrimitiveType::Kind::CharTy);

	if (!intTy_)
		intTy_ = new(*this) PrimitiveType(PrimitiveType::Kind::IntTy);

	if (!floatTy_)
		floatTy_ = new(*this) PrimitiveType(PrimitiveType::Kind::FloatTy);
}

ArrayType* ASTContext::getArrayTypeForType(Type * ty)
{
	auto lb = arrayTypes_.lower_bound(ty);
	if (lb != arrayTypes_.end() && !(arrayTypes_.key_comp()(ty, lb->first)))
	{
		// Key already exists, return lb->second.get()
		return lb->second;
	}
	else
	{
		// Key does not exists, insert & return.
		auto insertionResult = arrayTypes_.insert(lb,{ ty, new(*this) ArrayType(ty) });
		return insertionResult->second;
	}
}

LinearAllocator<>& ASTContext::getAllocator()
{
	return alloc_;
}

void ASTContext::reset()
{
	units_.clear();
	mainUnit_ = nullptr;
	voidTy_ = nullptr;
	intTy_ = nullptr;
	floatTy_ = nullptr;
	boolTy_ = nullptr;
	charTy_ = nullptr;
	stringTy_ = nullptr;
	arrayTypes_.clear();

	alloc_.reset();

	initBuiltinTypes();
}