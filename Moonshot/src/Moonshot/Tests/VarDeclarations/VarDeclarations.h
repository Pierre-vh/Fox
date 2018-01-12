#pragma once

#include "../ITest.h"
#include "../../Fox/AST/CompTime/Semantic/TypeCheck.h"
#include "../../Fox/AST/Runtime/Expr/RTExprVisitor.h"

namespace Moonshot 
{
	class VarDeclarations : public ITest
	{
		public:
			VarDeclarations();
			~VarDeclarations();

			virtual std::string getTestName() const override;
			virtual bool runTest(Context & context) override;
	};
}


