////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTCompStmt.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// AST representation for Compound statements.									
////------------------------------------------------------////

#pragma once

#include "IASTStmt.hpp"
#include <vector>

namespace Moonshot
{
	struct ASTCompStmt : public IASTStmt
	{
		public:
			ASTCompStmt() = default;

			virtual void accept(IVisitor & vis) override;

			std::vector<std::unique_ptr<IASTStmt>> statements_;
	};
}


