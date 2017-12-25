#pragma once
//utils
#include "../../../Common/Utils/Utils.h"
#include "../../../Common/Errors/Errors.h"
#include "../../../Common/Types/Types.h"

namespace Moonshot
{
	struct ASTExpr;
	struct ASTValue;

	struct ASTVarDeclStmt;
	class IVisitor
	{
		public:
			virtual ~IVisitor() = 0;

			inline virtual void visit(ASTExpr &node){}
			inline virtual void visit(ASTValue &node) {}

			inline virtual void visit(ASTVarDeclStmt &node) {}
	};
}


