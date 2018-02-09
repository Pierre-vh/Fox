////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTCompStmt.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTWhileLoop.hpp"

using namespace Moonshot;

ASTWhileLoop::~ASTWhileLoop()
{

}

void ASTWhileLoop::accept(IVisitor & vis)
{
	vis.visit(*this);
}

bool ASTWhileLoop::isValid() const
{
	return (expr_ && body_);
}
