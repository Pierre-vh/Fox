#pragma once

#include "../../src/Moonshot/Fox/Parser/Parser.h"
#include "../../src/Moonshot/Fox/Lexer/Lexer.h"
#include "../../src/Moonshot/Fox/AST/Nodes/IASTNode.h"
#include "../../src/Moonshot/Common/Utils/Utils.h"
#include "../../src/Moonshot/Common/Types/Types.h"

// Visitors
#include "../../src/Moonshot/Fox/AST/Visitor/Dumper/Dumper.h"
#include "../../src/Moonshot/Fox/AST/CompTime/Semantic/TypeCheck.h"
#include "../../src/Moonshot/Fox/AST/Runtime/Expr/RTExprVisitor.h"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <functional>
#include <memory>

// Components to test : 
// VDecl		-> Dumper
// ExprStmt		-> TypeCheck -> Dumper -> RTExpr
// Expr			-> Same as above

namespace Moonshot
{
	namespace Test_CommonUtilities
	{
		constexpr char kSpacerChar = (char)205;
		std::string printSpacer(std::size_t spacerSize = 36);

		constexpr char kTitleSpacerChar = char(254);
		constexpr std::size_t title_pad = 10; // the space to the left and right of the title
		void printTitle(const std::string& title);
	}
	namespace BasicTests
	{
		bool run_expressionTests		(const bool& printAST = true);
		bool run_expressionStmtTests	(const bool& printAST = true);
		bool run_varDeclStmtTests		(const bool& printAST = true);
	}
	typedef std::vector<std::unique_ptr<IVisitor> > TH_IVIS_VEC;
	typedef std::vector<std::unique_ptr<IRTVisitor> > TH_RTVIS_VEC;
	class TesterHelper
	{
		public:
			static std::vector<std::string> readFile_individualLines(const std::string & fp); // the filepath and if the tests are expected to fail or not.
			static std::string readFile(const std::string& fp);

			static bool standardTest(const std::vector<std::string> &strs,
				const std::function<std::unique_ptr<Moonshot::IASTNode>(Moonshot::Parser*)>& fn, // Signature of parsing functions
				const bool& shouldFail,						// The test should be successful or nah?
				const TH_IVIS_VEC& ct_vis,
				const TH_RTVIS_VEC& rt_vis = std::vector<std::unique_ptr<IRTVisitor> >()
			);

			static bool standardTest(const std::string &str,
				const std::function<std::unique_ptr<Moonshot::IASTNode>(Moonshot::Parser*)>& fn, // Signature of parsing functions
				const bool& shouldFail,						// The test should be successful or nah?
				const TH_IVIS_VEC& ct_vis,
				const TH_RTVIS_VEC& rt_vis = std::vector<std::unique_ptr<IRTVisitor> >()
			);
	};
}

