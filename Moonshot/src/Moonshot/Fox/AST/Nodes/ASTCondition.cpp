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

void ASTCondition::accept(IVisitor & vis)
{
	vis.visit(*this);
}