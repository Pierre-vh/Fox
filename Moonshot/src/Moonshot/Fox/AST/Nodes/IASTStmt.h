#pragma once

#include "IASTNode.h"

namespace Moonshot
{
	struct IASTStmt : public IASTNode // implements IASTNode, but doesn't implement its pure virtual functions, making this class pure virtual too.
	{
		public:
			IASTStmt();
			virtual ~IASTStmt() = 0;

		private:
			DISALLOW_COPY_AND_ASSIGN(IASTStmt)
	};
}

