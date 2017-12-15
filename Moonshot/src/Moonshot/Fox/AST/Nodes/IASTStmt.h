#pragma once

#include "IASTNode.h"

namespace Moonshot
{
	class IASTStmt : public IASTNode // implements IASTNode, but doesn't implement its pure virtual functions, making this class pure virtual too.
	{
		public:
			IASTStmt();
			virtual ~IASTStmt() = 0;
	};
}

