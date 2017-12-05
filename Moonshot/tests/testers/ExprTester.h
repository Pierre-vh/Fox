#pragma once

#include "ITest.h"

#include "../../src/Moonshot/Fox/Parser/Parser.h"
#include "../../src/Moonshot/Fox/Lexer/Lexer.h"
#include "../../src/Moonshot/Fox/AST/Nodes/ASTExpr.h"
#include "../../src/Moonshot/Fox/AST/Visitor/Semantic/TypeCheck.h"
#include "../../src/Moonshot/Fox/AST/Visitor/Dumper/Dumper.h"
#include "../../src/Moonshot/Common/Utils/Utils.h"

#include <sstream>
#include <fstream>

class ExprTester : public ITest
{
	public:
		ExprTester();
		~ExprTester();

		// Inherited via ITest
		virtual bool run() override;

		bool showAST = false;
	private:
		bool testFile(const std::string &fp, const bool &shouldFail = false);
		bool testStr(const std::string &str,const bool &shouldFailTC = false);
};

