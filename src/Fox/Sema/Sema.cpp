////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Sema.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
//	This file implements Sema methods that aren't tied to Expression,
//	Statements, Declarations or Types.
////------------------------------------------------------////

#include "Sema.hpp"

using namespace fox;

// Sema::Result
Sema::SemaResult Sema::SemaResult::Success(ASTNode node)
{
	return SemaResult(true, node);
}

Sema::SemaResult Sema::SemaResult::Failure()
{
	return SemaResult(false, nullptr);
}

bool Sema::SemaResult::wasSuccessful() const
{
	return success_;
}

Sema::SemaResult::operator bool() const
{
	return success_;
}

const ASTNode Sema::SemaResult::getReplacement() const
{
	return node_;
}

ASTNode Sema::SemaResult::getReplacement()
{
	return node_;
}

bool Sema::SemaResult::hasReplacement() const
{
	return (node_.getOpaque() != nullptr);
}
