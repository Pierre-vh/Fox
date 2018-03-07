////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IASTStmt.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Base Abstract class for Statement nodes.										
////------------------------------------------------------////

#pragma once

#include "Moonshot/Fox/AST/IVisitor.hpp"

namespace Moonshot
{
	struct IASTStmt 
	{
		public:
			IASTStmt() = default;
			virtual ~IASTStmt() = 0 {}
			virtual void accept(IVisitor& vis) = 0;
	};
}

