////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTReturnStmt.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////


#include "ASTReturnStmt.h"

using namespace Moonshot;

ASTReturnStmt::ASTReturnStmt(std::unique_ptr<IASTExpr> rtr_expr)
{
	expr_ = std::move(rtr_expr);
}

void ASTReturnStmt::accept(IVisitor & vis)
{
	vis.visit(*this);
}

bool ASTReturnStmt::hasExpr() const
{
	return (bool)expr_;
}
