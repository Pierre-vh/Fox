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

ConditionalStatement::ConditionalStatement(std::unique_ptr<IASTExpr> expr, std::unique_ptr<IASTStmt> stmt)
{
	expr_ = std::move(expr);
	stmt_ = std::move(stmt);
}

ConditionalStatement ConditionalStatement::resetAndReturnTmp()
{
	ConditionalStatement rtr(std::move(expr_), std::move(stmt_));
	expr_ = nullptr; stmt_ = nullptr;
	return rtr;
}

bool ConditionalStatement::isNull() const
{
	return (!stmt_ && !expr_);
}

bool ConditionalStatement::isComplete() const
{
	return expr_ && stmt_;
}
