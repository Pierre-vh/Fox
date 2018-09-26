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
	using PrimKind = PrimitiveType::Kind;

	theIntType_ = new(*this) PrimitiveType(PrimKind::IntTy);
	theFloatType_ = new(*this) PrimitiveType(PrimKind::FloatTy);
	theCharType_ = new(*this) PrimitiveType(PrimKind::CharTy);
	theBoolType_ = new(*this) PrimitiveType(PrimKind::BoolTy);
	theStringType_ = new(*this) PrimitiveType(PrimKind::StringTy);
	theVoidType_ = new(*this) PrimitiveType(PrimKind::VoidTy);
	theErrorType_ = new(*this) ErrorType();
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

Type ASTContext::getArrayTypeForType(Type ty)
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

Type ASTContext::getLValueTypeForType(Type ty)
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

void ASTContext::reset()
{
	units_.clear();
	mainUnit_ = nullptr;
	arrayTypes_.clear();

	allocator_.reset();
}

Type ASTContext::createSemaType(TypeBase* ty)
{
	return new(*this) SemaType(ty);
}