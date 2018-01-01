////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IVisitor.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Declaration of the abstract class IVisitor, used as a base for 
// every compile time visitor, and visitors in general (except runtime ones)
// see IRTVisitor for that.
//
// The class is abstract, but implements the visit method for each node.
// So, when you implement a visitor, you override only the functions you need,
// and the one you didn't implemented will default to the ones here, instead
// of throwing an error.
////------------------------------------------------------////

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


