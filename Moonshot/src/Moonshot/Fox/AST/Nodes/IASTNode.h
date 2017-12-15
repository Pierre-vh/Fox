#pragma once

#include "../../../Common/FValue/FValue.h"
#include "../Runtime/IRTVisitor.h"
#include "../../AST/Visitor/IVisitor.h"

#define VISIT_THIS vis->visit(this);

namespace Moonshot
{
	struct IASTNode
	{
		public :
			IASTNode();
			~IASTNode();
			virtual void accept(IVisitor *vis) = 0;
			virtual FVal accept(IRTVisitor *vis) = 0;
		private:
			IASTNode(const IASTNode& other);
			IASTNode& operator=(const IASTNode&);
	};
}

