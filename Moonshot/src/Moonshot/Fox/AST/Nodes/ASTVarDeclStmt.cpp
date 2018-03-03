////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTVarDeclStmt.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTVarDeclStmt.hpp"

using namespace Moonshot;

ASTVarDeclStmt::ASTVarDeclStmt(const var::varattr & attr, std::unique_ptr<IASTExpr> iExpr)
{
	if (attr)
	{
		vattr_ = attr;
		if (iExpr)						// if iexpr is valid, move it to our attribute.
			initExpr_ = std::move(iExpr);
	}
	else
		throw std::invalid_argument("Supplied an empty var::varattr object to the constructor.");
}

void ASTVarDeclStmt::accept(IVisitor& vis)
{
	vis.visit(*this);
}
