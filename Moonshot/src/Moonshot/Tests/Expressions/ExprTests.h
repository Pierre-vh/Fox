////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ExprTests.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Main tests for Expressions 
// This tests : Expression Lexing, Parsing, Typechecking and Evaluation (RTExprVisitor)
////------------------------------------------------------////

#pragma once

#include "../ITest.h"

#include "../../Fox/AST/CompTime/Semantic/TypeCheck.h"
#include "../../Fox/AST/Runtime/Expr/RTExprVisitor.h"

namespace Moonshot
{
	class ExprTests : public ITest
	{
		public:
			ExprTests();
			~ExprTests();

			virtual std::string getTestName() const override;
			virtual bool runTest(Context & context) override;
	};
}



