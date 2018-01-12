////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IRTVisitor.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// IRTVisitor is a abstract class used as a base for every Runtime Visitors.
// This will likely not be used a lot in the future, except for evaluating
// constant expression, to apply some kind of constant folding.
////------------------------------------------------------////
#pragma once
//utils
#include "../../../Common/Utils/Utils.h"
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
			inline virtual FVal visit(ASTLiteral &node)			{ return FVal(); }

			inline virtual FVal visit(ASTVarDeclStmt &node)			{ return FVal(); }
			inline virtual FVal visit(ASTVarCall& node)				{ return FVal(); }
	};
}