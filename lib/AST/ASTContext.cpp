////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTContext.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Fox/AST/ASTContext.hpp"

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
	return theIntTy_;
}

PrimitiveType* ASTContext::getFloatType()
{
	return theFloatTy_;
}

PrimitiveType* ASTContext::getCharType()
{
	return theCharTy_;
}

PrimitiveType* ASTContext::getBoolType()
{
	return theBoolTy_;
}

PrimitiveType* ASTContext::getStringType()
{
	return theStringTy_;
}

PrimitiveType* ASTContext::getVoidType()
{
	return theVoidTy_;
}

ErrorType* ASTContext::getErrorType()
{
	return theErrorTy_;
}

void ASTContext::initBuiltinTypes()
{
	if (!theVoidTy_)
		theVoidTy_ = new(*this) PrimitiveType(PrimitiveType::Kind::VoidTy);

	if (!theBoolTy_)
		theBoolTy_ = new(*this) PrimitiveType(PrimitiveType::Kind::BoolTy);

	if (!theStringTy_)
		theStringTy_ = new(*this) PrimitiveType(PrimitiveType::Kind::StringTy);

	if (!theCharTy_)
		theCharTy_ = new(*this) PrimitiveType(PrimitiveType::Kind::CharTy);

	if (!theIntTy_)
		theIntTy_ = new(*this) PrimitiveType(PrimitiveType::Kind::IntTy);

	if (!theFloatTy_)
		theFloatTy_ = new(*this) PrimitiveType(PrimitiveType::Kind::FloatTy);

	if (!theErrorTy_)
		theErrorTy_ = new(*this) ErrorType();
}

ArrayType* ASTContext::getArrayTypeForType(TypeBase * ty)
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

LValueType* ASTContext::getLValueTypeForType(TypeBase * ty)
{
	auto lb = lvalueTypes_.lower_bound(ty);
	if (lb != lvalueTypes_.end() && !(lvalueTypes_.key_comp()(ty, lb->first)))
	{
		// Key already exists, return lb->second.get()
		return lb->second;
	}
	else
	{
		// Key does not exists, insert & return.
		auto insertionResult = lvalueTypes_.insert(lb, { ty, new(*this) LValueType(ty) });
		return insertionResult->second;
	}
}

LinearAllocator<>& ASTContext::getAllocator()
{
	return allocator_;
}

void ASTContext::reset()
{
	units_.clear();
	mainUnit_ = nullptr;
	theVoidTy_ = nullptr;
	theIntTy_ = nullptr;
	theFloatTy_ = nullptr;
	theBoolTy_ = nullptr;
	theCharTy_ = nullptr;
	theStringTy_ = nullptr;
	arrayTypes_.clear();

	allocator_.reset();

	initBuiltinTypes();
}

SemaType* ASTContext::createSemaType(TypeBase* ty)
{
	return new(*this) SemaType(ty);
}