#pragma once
//utils
#include "../../../Common/Utils/Utils.h"
#include "../../../Common/Errors/Errors.h"
#include "../../../Common/Types/Types.h"

#include "../Nodes/NodesForwardDeclaration.h"


namespace Moonshot
{
	NODE_FORWARD_DECLARATION
	class IVisitor
	{
		public:
			virtual ~IVisitor() = 0;

			inline virtual void visit(ASTExpr &node)			{}
			inline virtual void visit(ASTRawValue &node)		{}

			inline virtual void visit(ASTVarDeclStmt &node)		{}
			inline virtual void visit(ASTVarCall& node)			{}
	};
}


