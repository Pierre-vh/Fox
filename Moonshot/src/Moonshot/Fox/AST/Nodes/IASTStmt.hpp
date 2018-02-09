////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IASTStmt.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Base Abstract class for Statement nodes.										
////------------------------------------------------------////

#pragma once

#include "IASTNode.hpp"

namespace Moonshot
{
	struct IASTStmt : public IASTNode 
	{
		public:
			IASTStmt() = default;
			virtual ~IASTStmt() = 0 {}
	};
}

