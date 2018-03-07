////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IASTExpr.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Base abstract class for Expr nodes.									
////------------------------------------------------------////

#pragma once

#include "Moonshot/Fox/AST/Nodes/IASTStmt.hpp"
#include "Moonshot/Common/Types/Types.hpp"

namespace Moonshot
{
	struct IASTExpr : public IASTStmt
	{
		public:
			IASTExpr() = default;
			inline virtual ~IASTExpr() = 0 {}
			virtual void accept(IVisitor& vis) = 0;
			FoxType resultType_ = 0; // The planified result type of the expression after execution. this is set by the typechecker.
	};
}