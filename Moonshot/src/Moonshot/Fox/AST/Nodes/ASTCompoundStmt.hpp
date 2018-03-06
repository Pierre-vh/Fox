////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTCompoundStmt.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// AST representation for Compound statements.									
////------------------------------------------------------////

#pragma once

#include "IASTStmt.hpp"
#include <vector>
#include <memory>

namespace Moonshot
{
	struct ASTCompoundStmt : public IASTStmt
	{
		public:
			ASTCompoundStmt() = default;

			virtual void accept(IVisitor & vis) override;

			std::vector<std::unique_ptr<IASTStmt>> statements_;
	};
}


