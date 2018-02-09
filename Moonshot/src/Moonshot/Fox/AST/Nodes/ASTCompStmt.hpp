////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTCompStmt.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// AST representation for Compound statements.									
////------------------------------------------------------////

#pragma once

#include "IASTNode.hpp"
#include "IASTStmt.hpp"
#include <vector>

namespace Moonshot
{
	struct ASTCompStmt : public IASTStmt
	{
		public:
			ASTCompStmt();
			~ASTCompStmt();

			virtual void accept(IVisitor & vis) override;

			std::vector<std::unique_ptr<IASTStmt>> statements_;
	};
}


