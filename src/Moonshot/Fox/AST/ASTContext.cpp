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

BuiltinType * ASTContext::getBuiltinIntType()
{
	return &builtinInt_;
}

BuiltinType * ASTContext::getBuiltinFloatType()
{
	return &builtinFloat_;
}

BuiltinType * ASTContext::getBuiltinCharType()
{
	return &builtinChar_;
}

BuiltinType * ASTContext::getBuiltinBoolType()
{
	return &builtinBool_;
}

BuiltinType * ASTContext::getBuiltinStringType()
{
	return &builtinString_;
}

BuiltinType * Moonshot::ASTContext::getBuiltinVoidType()
{
	return &builtinVoid_;
}

void ASTContext::initBuiltinTypes()
{
	builtinVoid_.setBuiltinKind(BuiltinType::Kind::VoidTy);

	builtinInt_.setBuiltinKind(BuiltinType::Kind::IntTy);
	builtinFloat_.setBuiltinKind(BuiltinType::Kind::FloatTy);
	builtinBool_.setBuiltinKind(BuiltinType::Kind::BoolTy);

	builtinString_.setBuiltinKind(BuiltinType::Kind::StringTy);
	builtinChar_.setBuiltinKind(BuiltinType::Kind::CharTy);
}