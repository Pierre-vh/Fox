#pragma once
//utils
#include "../../../Common/Utils/Utils.h"
#include "../../../Common/Errors/Errors.h"
#include "../../../Common/FValue/FValue.h"

#define IRTVIS_DEFAULTBODY {return FVal();}

namespace Moonshot
{
	struct ASTExpr;
	struct ASTValue;
	struct ASTVarDeclStmt;
	class IRTVisitor
	{
		public :
			virtual ~IRTVisitor() = 0;

			inline virtual FVal visit(ASTExpr *node)			IRTVIS_DEFAULTBODY
			inline virtual FVal visit(ASTValue *node)			IRTVIS_DEFAULTBODY

			inline virtual FVal visit(ASTVarDeclStmt *node)		IRTVIS_DEFAULTBODY
	};
}