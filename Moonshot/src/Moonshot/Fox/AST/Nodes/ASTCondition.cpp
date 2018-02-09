////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTCondition.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTCondition.hpp"

using namespace Moonshot;

ASTCondition::ASTCondition()
{
}


ASTCondition::~ASTCondition()
{
}

void ASTCondition::accept(IVisitor & vis)
{
	vis.visit(*this);
}

bool ASTCondition::hasElse() const
{
	return (else_block_ ? true : false);
}

bool ASTCondition::hasElif() const
{
	return conditional_blocks_.size() > 1; 
}

bool ASTCondition::isValid() const
{
	return false;
}
