////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : VarDeclarations.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// blah blah blah											
////------------------------------------------------------////

#pragma once

#include "../ITest.hpp"
#include "../../Fox/Semantic/TypeCheck.hpp"
#include "../../Fox/Eval/Expr/RTExprVisitor.hpp"

namespace Moonshot::Test
{
	class VarDeclarations : public ITest
	{
		public:
			VarDeclarations() = default;

			virtual std::string getTestName() const override;
			virtual bool runTest(Context & context) override;
	};
}


