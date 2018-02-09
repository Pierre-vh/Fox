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

	struct ASTCondition : public IASTStmt
	{
		public:
			ASTCondition();
			~ASTCondition();

			// "CondBlock" = Expression + Compound Statement
			typedef std::pair<std::unique_ptr<IASTExpr>, std::unique_ptr<IASTStmt>> CondBlock;

			virtual void accept(IVisitor & vis) override;

			bool hasElse() const;
			bool hasElif() const;
			bool isValid() const;

			std::vector<CondBlock> conditional_blocks_; // First one is the if, all others are the elifs
			std::unique_ptr<IASTStmt> else_block_; // final else.
	};
}
