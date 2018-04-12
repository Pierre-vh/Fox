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

TypePtr ASTContext::getBuiltinIntType()
{
	return builtinInt_;
}

TypePtr ASTContext::getBuiltinFloatType()
{
	return builtinFloat_;
}

TypePtr ASTContext::getBuiltinCharType()
{
	return builtinChar_;
}

TypePtr ASTContext::getBuiltinBoolType()
{
	return builtinBool_;
}

TypePtr ASTContext::getBuiltinStringType()
{
	return builtinString_;
}

TypePtr ASTContext::getBuiltinVoidType()
{
	return builtinVoid_;
}

void ASTContext::initBuiltinTypes()
{
	builtinVoid_	= std::make_shared<BuiltinType>(BuiltinType::Kind::VoidTy);

	builtinInt_		= std::make_shared<BuiltinType>(BuiltinType::Kind::IntTy);
	builtinFloat_	= std::make_shared<BuiltinType>(BuiltinType::Kind::FloatTy);
	builtinBool_	= std::make_shared<BuiltinType>(BuiltinType::Kind::BoolTy);

	builtinString_	= std::make_shared<BuiltinType>(BuiltinType::Kind::StringTy);
	builtinChar_	= std::make_shared<BuiltinType>(BuiltinType::Kind::CharTy);
}