////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTWhileLoop.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTWhileLoop.hpp"

using namespace Moonshot;

void ASTWhileLoop::accept(IVisitor & vis)
{
	vis.visit(*this);
}
