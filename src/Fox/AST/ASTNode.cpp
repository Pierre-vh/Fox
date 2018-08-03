////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTNode.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTNode.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

SourceRange ASTNode::getRange() const
{
	if (is<Expr>())
		return get<Expr>()->getRange();
	if (is<Decl>())
		return get<Decl>()->getRange();
	if (is<Stmt>())
		return get<Stmt>()->getRange();
	fox_unreachable("Unsupported node");
}

SourceLoc ASTNode::getBegLoc() const
{
	return getRange().getBegin();
}

SourceLoc ASTNode::getEndLoc() const
{
	return getRange().getEnd();
}
