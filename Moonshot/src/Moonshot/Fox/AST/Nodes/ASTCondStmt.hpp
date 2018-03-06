////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTCondStmt.hpp											
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
	struct ASTCondStmt : public IASTStmt
	{
		public:
			ASTCondStmt() = default;

			virtual void accept(IVisitor & vis) override;

			std::unique_ptr<IASTExpr> cond_;
			std::unique_ptr<IASTStmt> then_; 
			std::unique_ptr<IASTStmt> else_;
	};
}
