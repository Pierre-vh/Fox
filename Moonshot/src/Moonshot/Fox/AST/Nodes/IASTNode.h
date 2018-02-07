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
#include "../../AST/Visitor/IVisitor.h"

namespace Moonshot
{
	struct IASTNode
	{
		public :
			IASTNode();
			~IASTNode();
			virtual void accept(IVisitor& vis) = 0;
	};
}

