////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ParsingResult.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Implements UnitParsingResult methods
////------------------------------------------------------////

#include "ParsingResult.hpp"
#include "Moonshot/Fox/AST/ASTUnit.hpp"

using namespace Moonshot;

// UnitParsingResult
UnitParsingResult::UnitParsingResult(std::unique_ptr<ASTUnit> parsedUnit)
{
	unit = std::move(parsedUnit);
}

UnitParsingResult::operator bool() const
{
	return (bool)unit;
}