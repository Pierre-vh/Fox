////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTCondition.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// AST nodes for conditions.								
////------------------------------------------------------////

#pragma once

#include "IASTStmt.hpp"
#include "ASTExpr.hpp"
#include <tuple>
#include <vector>

namespace Moonshot
{
	struct ConditionalStatement
	{
		// This is a simple struct that eases the storage of a expression + statement pair.
		ConditionalStatement() = default;
		// /!\ This constructor moves the attributes passed as argument.
		ConditionalStatement(std::unique_ptr<IASTExpr> &expr, std::unique_ptr<IASTStmt> &stmt);
		ConditionalStatement resetAndReturnTmp();
		// Contains an expression and a statement.
		std::unique_ptr<IASTExpr> expr_;
		std::unique_ptr<IASTStmt> stmt_;

		bool isNull() const; // returns true if !stmt_ && !expr_
		bool isComplete() const; // returns true if stmt_ && expr_
	};
	struct ASTCondition : public IASTStmt
	{
		public:
			ASTCondition() = default;

			virtual void accept(IVisitor & vis) override;

			std::vector<ConditionalStatement> conditional_stmts_; // First one is the if, all others are the elifs
			std::unique_ptr<IASTStmt> else_stmt_; // final else.
	};
}
