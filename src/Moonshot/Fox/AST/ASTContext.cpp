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

ASTUnit * ASTContext::getMainUnit()
{
	return mainUnit_;
}

ASTUnit * ASTContext::setMainUnit(std::unique_ptr<ASTUnit> unit)
{
	mainUnit_ = addUnit(std::move(unit));
	return mainUnit_;
}

ASTUnit * ASTContext::addUnit(std::unique_ptr<ASTUnit> unit)
{
	units_.emplace_back(std::move(unit));
	return units_.back().get();
}

IdentifierTable & ASTContext::identifierTable()
{
	return idents_;
}

PrimitiveType* ASTContext::getPrimitiveIntType()
{
	return primitiveIntTy_.get();
}

PrimitiveType* ASTContext::getPrimitiveFloatType()
{
	return primitiveFloatTy_.get();
}

PrimitiveType* ASTContext::getPrimitiveCharType()
{
	return primitiveCharTy_.get();
}

PrimitiveType* ASTContext::getPrimitiveBoolType()
{
	return primitiveBoolTy_.get();
}

PrimitiveType* ASTContext::getPrimitiveStringType()
{
	return primitiveStringTy.get();
}

PrimitiveType* ASTContext::getPrimitiveVoidType()
{
	return primitiveVoidTy_.get();
}

void ASTContext::initBuiltinTypes()
{
	primitiveVoidTy_	= std::make_unique<PrimitiveType>(PrimitiveType::Kind::VoidTy);

	primitiveIntTy_		= std::make_unique<PrimitiveType>(PrimitiveType::Kind::IntTy);
	primitiveFloatTy_	= std::make_unique<PrimitiveType>(PrimitiveType::Kind::FloatTy);
	primitiveBoolTy_	= std::make_unique<PrimitiveType>(PrimitiveType::Kind::BoolTy);

	primitiveStringTy	= std::make_unique<PrimitiveType>(PrimitiveType::Kind::StringTy);
	primitiveCharTy_	= std::make_unique<PrimitiveType>(PrimitiveType::Kind::CharTy);
}