////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ParsingResult.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Implements UnitParsingResult and ResyncResult methods.
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

// ResyncResult
ResyncResult::ResyncResult(const bool& succeeded, const bool& onRequestedToken)
{
	resynced_ = succeeded;
	resyncedOnRequestedToken_ = onRequestedToken;
}

ResyncResult::operator bool()
{
	return hasRecovered();
}

bool ResyncResult::hasRecovered() const
{
	return resynced_;
}

bool ResyncResult::hasRecoveredOnRequestedToken() const
{
	return resynced_ && resyncedOnRequestedToken_;
}