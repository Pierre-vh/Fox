////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IASTNode.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Base abstract class for nodes.											
////------------------------------------------------------////

#pragma once

#include "../../../Common/Types/Types.hpp"
#include "../../AST/Visitor/IVisitor.hpp"

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

