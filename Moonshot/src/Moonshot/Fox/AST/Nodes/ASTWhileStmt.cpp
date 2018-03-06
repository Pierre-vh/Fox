////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTWhileStmt.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTWhileStmt.hpp"

using namespace Moonshot;

void ASTWhileStmt::accept(IVisitor & vis)
{
	vis.visit(*this);
}
