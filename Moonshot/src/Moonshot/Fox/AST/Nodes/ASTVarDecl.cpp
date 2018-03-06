////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTVarDecl.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTVarDecl.hpp"

using namespace Moonshot;

ASTVarDecl::ASTVarDecl(const FoxVariableAttr & attr, std::unique_ptr<IASTExpr> iExpr)
{
	if (attr)
	{
		vattr_ = attr;
		if (iExpr)						// if iexpr is valid, move it to our attribute.
			initExpr_ = std::move(iExpr);
	}
	else
		throw std::invalid_argument("Supplied an empty FoxVariableAttr object to the constructor.");
}

void ASTVarDecl::accept(IVisitor& vis)
{
	vis.visit(*this);
}
