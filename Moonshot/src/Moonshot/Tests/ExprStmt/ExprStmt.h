////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ExprStmt.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//
// Expression statements tests
// Uses test files /res/tests/exprstmt/exprstmt_[bad | correct].fox
//
////------------------------------------------------------////

#pragma once

#include "../ITest.h"

#include "../../Fox/AST/Visitor/Semantic/TypeCheck.h"
#include "../../Fox/AST/Visitor/Eval/Expr/RTExprVisitor.h"

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

