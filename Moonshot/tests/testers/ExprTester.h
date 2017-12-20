#pragma once

#include "ITest.h"

#include "../../src/Moonshot/Fox/AST/Nodes/ASTExpr.h"
#include "../../src/Moonshot/Fox/AST/CompTime/Semantic/TypeCheck.h"
#include "../../src/Moonshot/Common/Utils/Utils.h"
#include "../../src/Moonshot/Fox/AST/Runtime/Expr/RTExprVisitor.h"


class ExprTester : public ITest
{
	public:
		ExprTester();
		~ExprTester();

		// Inherited via ITest
		virtual bool run() override;
		
	protected:
		bool testStr(const std::string &str, const bool &shouldFailTC = false);
};

