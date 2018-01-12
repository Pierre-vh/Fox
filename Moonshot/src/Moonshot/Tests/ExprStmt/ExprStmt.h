////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ExprStmtTest.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//
// Expression statements tests
// Uses test files /res/tests/exprstmt/exprstmt_[bad | correct].fox
//
////------------------------------------------------------////

#pragma once

#include "../ITest.h"

#include "../../Fox/AST/CompTime/Semantic/TypeCheck.h"
#include "../../Fox/AST/Runtime/Expr/RTExprVisitor.h"

namespace Moonshot
{
	class ExprStmtTest : public ITest
	{
		public:
			ExprStmtTest();
			~ExprStmtTest();

			virtual std::string getTestName() const override;
			virtual bool runTest(Context & context) override;
	};
}

