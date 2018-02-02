////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTWhileLoop.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// The AST Node for While loops.
////------------------------------------------------------////

#pragma once

#include "IASTStmt.h"
#include "ASTExpr.h"
#include "ASTCompStmt.h"

namespace Moonshot
{
	struct ASTWhileLoop : public IASTStmt
	{
		public:
			ASTWhileLoop() = default;
			~ASTWhileLoop();

			virtual void accept(IVisitor & vis) override;

			bool isValid() const;

			std::unique_ptr<IASTExpr> expr_;
			std::unique_ptr<ASTCompStmt> body_;
	};
}
