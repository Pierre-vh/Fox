////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTStmt.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTStmt.hpp"
#include "ASTExpr.hpp"

using namespace Moonshot;

void ASTNullStmt::accept(IVisitor& vis)
{ 
	vis.visit(*this);
}

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

void ASTCondStmt::accept(IVisitor & vis)
{
	vis.visit(*this);
}

void ASTCompoundStmt::accept(IVisitor & vis)
{
	vis.visit(*this);
}

void ASTWhileStmt::accept(IVisitor & vis)
{
	vis.visit(*this);
}
