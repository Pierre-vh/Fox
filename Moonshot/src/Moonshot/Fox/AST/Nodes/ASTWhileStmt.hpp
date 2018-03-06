////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTWhileStmt.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// The AST Node for While loops.
////------------------------------------------------------////

#pragma once

#include "IASTStmt.hpp"
#include "IASTExpr.hpp"
#include <memory>

namespace Moonshot
{
	struct IASTExpr;
	struct ASTWhileStmt : public IASTStmt
	{
		public:
			ASTWhileStmt() = default;

			virtual void accept(IVisitor & vis) override;

			std::unique_ptr<IASTExpr> expr_;
			std::unique_ptr<IASTStmt> body_;
	};
}
