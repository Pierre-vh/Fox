#pragma once
//utils
#include "../../../Common/Utils/Utils.h"
#include "../../../Common/Errors/Errors.h"
#include "../../../Common/FValue/FValue.h"

namespace Moonshot
{
	struct ASTExpr;
	struct ASTValue;
	struct ASTVarDeclStmt;
	class IRTVisitor
	{
		public :
			virtual ~IRTVisitor() = 0;

			inline virtual FVal visit(ASTExpr *node)		{ return FVal(); }
			inline virtual FVal visit(ASTValue *node)		{ return FVal(); }

			inline virtual FVal visit(ASTVarDeclStmt *node) { return FVal(); }
	};
}