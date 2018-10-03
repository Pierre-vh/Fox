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
	initBuiltins();
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
	return theIntType_;
}

PrimitiveType* ASTContext::getFloatType()
{
	return theFloatType_;
}

PrimitiveType* ASTContext::getBoolType()
{
	return theBoolType_;
}

PrimitiveType* ASTContext::getStringType()
{
	return theStringType_;
}

PrimitiveType* ASTContext::getCharType()
{
	return theCharType_;
}

PrimitiveType* ASTContext::getVoidType()
{
	return theVoidType_;
}

ErrorType* ASTContext::getErrorType()
{
	return theErrorType_;
}

ArrayType* ASTContext::getArrayTypeForType(Type ty)
{
	auto lb = arrayTypes_.lower_bound(ty.getPtr());
	if (lb != arrayTypes_.end() && !(arrayTypes_.key_comp()(ty.getPtr(), lb->first)))
	{
		// Key already exists, return lb->second.get()
		return lb->second;
	}
	else
	{
		// Key does not exists, insert & return.
		auto insertionResult = arrayTypes_.insert(lb,{ ty.getPtr() , new(*this) ArrayType(ty.getPtr()) });
		return insertionResult->second;
	}
}

LValueType* ASTContext::getLValueTypeForType(Type ty)
{
	auto lb = lvalueTypes_.lower_bound(ty.getPtr());
	if (lb != lvalueTypes_.end() && !(lvalueTypes_.key_comp()(ty.getPtr(), lb->first)))
	{
		// Key already exists, return lb->second.get()
		return lb->second;
	}
	else
	{
		// Key does not exists, insert & return.
		auto insertionResult = lvalueTypes_.insert(lb, { ty.getPtr(), new(*this) LValueType(ty.getPtr()) });
		return insertionResult->second;
	}
}

LinearAllocator<>& ASTContext::getAllocator()
{
	return allocator_;
}

LinearAllocator<>& ASTContext::getCSAllocator()
{
	return csAllocator_;
}

void ASTContext::reset()
{
	units_.clear();
	mainUnit_ = nullptr;
	arrayTypes_.clear();

	theIntType_ = nullptr;
	theFloatType_ = nullptr;
	theCharType_ = nullptr;
	theBoolType_ = nullptr;
	theStringType_ = nullptr;
	theVoidType_ = nullptr;
	theErrorType_ = nullptr;

	allocator_.reset();
	csAllocator_.reset();
}

void ASTContext::freeCS()
{
	csAllocator_.reset();
}

void ASTContext::initBuiltins()
{
	using PrimKind = PrimitiveType::Kind;

	theIntType_ = new(*this) PrimitiveType(PrimKind::IntTy);
	theFloatType_ = new(*this) PrimitiveType(PrimKind::FloatTy);
	theCharType_ = new(*this) PrimitiveType(PrimKind::CharTy);
	theBoolType_ = new(*this) PrimitiveType(PrimKind::BoolTy);
	theStringType_ = new(*this) PrimitiveType(PrimKind::StringTy);
	theVoidType_ = new(*this) PrimitiveType(PrimKind::VoidTy);
	theErrorType_ = new(*this) ErrorType();
}