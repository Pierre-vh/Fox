////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IASTDeclaration.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Base abstract class for nodes.											
////------------------------------------------------------////

#pragma once

#include "Moonshot/Fox/AST/Nodes/IASTNode.hpp"

namespace Moonshot
{
	struct IASTDeclaration : public IASTNode
	{
		public:
			IASTDeclaration() = default;
			virtual ~IASTDeclaration() = 0 {}
	};
}

