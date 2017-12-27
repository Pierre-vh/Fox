#pragma once
//utils
#include "../../../Common/Utils/Utils.h"
#include "../../../Common/Errors/Errors.h"
#include "../../../Common/Types/Types.h"

#include "../Nodes/NodesForwardDeclaration.h"

namespace Moonshot
{
	NODE_FORWARD_DECLARATION
	class IRTVisitor
	{
		public :
			virtual ~IRTVisitor() = 0;

			inline virtual FVal visit(ASTExpr &node)				{ return FVal(); }
			inline virtual FVal visit(ASTRawValue &node)			{ return FVal(); }

			inline virtual FVal visit(ASTVarDeclStmt &node)			{ return FVal(); }
			inline virtual FVal visit(ASTVarCall& node)				{ return FVal(); }
	};
}