#pragma once

#include "IASTNode.h"
#include "IASTStmt.h"
#include <vector>

namespace Moonshot
{
	struct ASTCompStmt : public IASTNode
	{
		public:
			ASTCompStmt();
			~ASTCompStmt();

			virtual void accept(IVisitor & vis) override;
			virtual FVal accept(IRTVisitor & vis) override;

			std::vector<std::unique_ptr<IASTStmt>> statements_;
	};
}


