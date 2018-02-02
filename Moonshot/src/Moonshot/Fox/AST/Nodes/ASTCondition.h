////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTCondition.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// AST nodes for conditions.								
////------------------------------------------------------////

#pragma once

#include "IASTStmt.h"
#include "ASTCompStmt.h"
#include "ASTExpr.h"
#include <tuple>

namespace Moonshot
{

	struct ASTCondition : public IASTStmt
	{
		public:
			ASTCondition();
			~ASTCondition();

			// "CondBlock" = Expression + Compound Statement
			typedef std::pair<std::unique_ptr<IASTExpr>, std::unique_ptr<ASTCompStmt>> CondBlock;

			virtual void accept(IVisitor & vis) override;

			bool hasElse() const;
			bool hasElif() const;
			bool isValid() const;

			std::vector<CondBlock> conditional_blocks_; // First one is the if, all others are the elifs
			std::unique_ptr<ASTCompStmt> else_block_; // final else.
	};
}
