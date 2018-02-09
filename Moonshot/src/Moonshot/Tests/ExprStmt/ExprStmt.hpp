////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ExprStmt.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//
// Expression statements tests
// Uses test files /res/tests/exprstmt/exprstmt_[bad | correct].fox
//
////------------------------------------------------------////

#pragma once

#include "../ITest.hpp"

#include "../../Fox/AST/Visitor/Semantic/TypeCheck.hpp"
#include "../../Fox/AST/Visitor/Eval/Expr/RTExprVisitor.hpp"

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

