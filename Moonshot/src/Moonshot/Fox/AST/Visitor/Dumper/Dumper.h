#pragma once
// base class
#include "../IVisitor.h"
// FVal Utilities
#include "../../../../Common/FValue/FValue.h"
// Include nodes
#include "../../Nodes/ASTExpr.h"

namespace Moonshot
{
	class Dumper : public IVisitor
	{
		public:
			Dumper();
			~Dumper();

			virtual void visit(ASTExpr * node) override;
			virtual void visit(ASTValue * node) override;

		private:
			std::string tabs() const;
			unsigned int tabcount = 1;
	};
}


