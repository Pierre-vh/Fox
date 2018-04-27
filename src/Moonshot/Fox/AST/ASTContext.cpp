////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTContext.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTContext.hpp"

using namespace Moonshot;

ASTContext::ASTContext()
{
	// Init builtin types
	initBuiltinTypes();
}

UnitDecl * ASTContext::getMainUnit()
{
	return mainUnit_;
}

UnitDecl * ASTContext::setMainUnit(std::unique_ptr<UnitDecl> unit)
{
	mainUnit_ = addUnit(std::move(unit));
	return mainUnit_;
}

UnitDecl * ASTContext::addUnit(std::unique_ptr<UnitDecl> unit)
{
	units_.emplace_back(std::move(unit));
	return units_.back().get();
}

const PrimitiveType* ASTContext::getPrimitiveIntType() const
{
	return primitiveIntTy_.get();
}

const PrimitiveType* ASTContext::getPrimitiveFloatType() const
{
	return primitiveFloatTy_.get();
}

const PrimitiveType* ASTContext::getPrimitiveCharType() const
{
	return primitiveCharTy_.get();
}

const PrimitiveType* ASTContext::getPrimitiveBoolType() const
{
	return primitiveBoolTy_.get();
}

const PrimitiveType* ASTContext::getPrimitiveStringType() const
{
	return primitiveStringTy_.get();
}

const PrimitiveType* ASTContext::getPrimitiveVoidType() const
{
	return primitiveVoidTy_.get();
}

void ASTContext::initBuiltinTypes()
{
	primitiveVoidTy_	= std::make_unique<PrimitiveType>(PrimitiveType::Kind::VoidTy);

	primitiveIntTy_		= std::make_unique<PrimitiveType>(PrimitiveType::Kind::IntTy);
	primitiveFloatTy_	= std::make_unique<PrimitiveType>(PrimitiveType::Kind::FloatTy);
	primitiveBoolTy_	= std::make_unique<PrimitiveType>(PrimitiveType::Kind::BoolTy);

	primitiveStringTy_	= std::make_unique<PrimitiveType>(PrimitiveType::Kind::StringTy);
	primitiveCharTy_	= std::make_unique<PrimitiveType>(PrimitiveType::Kind::CharTy);
}

const ArrayType * ASTContext::getArrayTypeForType(const Type * ty)
{
	// Effective STL, Item 24 by Scott Meyers : https://stackoverflow.com/a/101980
	auto lb = arrayTypes_.lower_bound(ty);
	if (lb != arrayTypes_.end() && !(arrayTypes_.key_comp()(ty, lb->first)))
	{
		// Key already exists, return lb->second.get()
		return (lb->second).get();
	}
	else
	{
		// Key does not exists, insert & return.
		auto insertionResult = arrayTypes_.insert(lb,{ ty, std::make_unique<ArrayType>(ty) });
		return (insertionResult->second).get();
	}
}