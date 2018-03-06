////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTCompoundStmt.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTCompoundStmt.hpp"

using namespace Moonshot;

void ASTCompoundStmt::accept(IVisitor & vis)
{
	vis.visit(*this);
}