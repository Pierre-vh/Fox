////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTReturnStmt.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// AST Node for the return statement.			
////------------------------------------------------------////

#pragma once

#include "IASTStmt.hpp"
#include "ASTExpr.hpp"
#include <memory>

namespace Moonshot
{
	struct ASTReturnStmt : public IASTStmt	// A null statement, that doesn't do anything. It's going to be ignored most of the time, isn't that sad?
	{
		ASTReturnStmt() = default;
		ASTReturnStmt(std::unique_ptr<IASTExpr> rtr_expr);

		virtual void accept(IVisitor& vis) override;

		bool hasExpr() const;
		std::unique_ptr<IASTExpr> expr_;
	};
}