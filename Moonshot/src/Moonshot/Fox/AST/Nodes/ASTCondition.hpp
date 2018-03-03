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
#include <vector>
#include <memory>

namespace Moonshot
{
	struct ASTCondition : public IASTStmt
	{
		public:
			ASTCondition() = default;

			virtual void accept(IVisitor & vis) override;

			std::unique_ptr<IASTExpr> condition_expr_;
			std::unique_ptr<IASTStmt> condition_stmt_; // First one is the if, all others are the elifs
			std::unique_ptr<IASTStmt> else_stmt_; // final else.
	};
}
