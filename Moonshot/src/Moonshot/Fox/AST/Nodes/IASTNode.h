////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IASTNode.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Base abstract class for nodes.											
////------------------------------------------------------////

#pragma once

#include "../../../Common/Types/Types.h"
#include "../Runtime/IRTVisitor.h"
#include "../../AST/Visitor/IVisitor.h"
#include "../../../Common/Macros.h"

namespace Moonshot
{
	struct IASTNode
	{
		public :
			IASTNode();
			~IASTNode();
			virtual void accept(IVisitor& vis) = 0;
			virtual FVal accept(IRTVisitor& vis) = 0;
		private:
			DISALLOW_COPY_AND_ASSIGN(IASTNode)
	};
}

