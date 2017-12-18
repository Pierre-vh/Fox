#pragma once

#include "../../../Common/Types/Types.h"
#include "../Runtime/IRTVisitor.h"
#include "../../AST/Visitor/IVisitor.h"

namespace Moonshot
{
	struct IASTNode
	{
		public :
			IASTNode();
			~IASTNode();
			virtual void accept(IVisitor& vis) = 0;
			virtual FVal accept(IRTVisitor& vis) = 0;
		private:
			IASTNode(const IASTNode& other);
			IASTNode& operator=(const IASTNode&);
	};
}

