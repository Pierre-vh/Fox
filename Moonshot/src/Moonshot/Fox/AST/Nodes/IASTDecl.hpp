////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IASTDecl.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Base abstract class for nodes.											
////------------------------------------------------------////

#pragma once

#include "IASTStmt.hpp"

namespace Moonshot
{
	struct IASTDecl : public IASTStmt
	{
		public:
			IASTDecl() = default;
			virtual ~IASTDecl() = 0 {}
			virtual void accept(IVisitor& vis) = 0;
	};
}

