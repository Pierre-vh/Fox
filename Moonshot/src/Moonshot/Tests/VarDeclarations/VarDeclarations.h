////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : VarDeclarations.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// blah blah blah											
////------------------------------------------------------////

#pragma once

#include "../ITest.h"
#include "../../Fox/AST/Visitor/Semantic/TypeCheck.h"
#include "../../Fox/AST/Visitor/Eval/Expr/RTExprVisitor.h"

namespace Moonshot 
{
	class VarDeclarations : public ITest
	{
		public:
			VarDeclarations() = default;
			~VarDeclarations();

			virtual std::string getTestName() const override;
			virtual bool runTest(Context & context) override;
	};
}


