////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ExprTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ExprTests.hpp"
#include "Moonshot/Common/Types/TypesUtils.hpp"
#include "Moonshot/Fox/AST/Visitor/Dumper/Dumper.hpp"
#include "Moonshot/Fox/AST/Visitor/Semantic/TypeCheck.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTExpr.hpp"

using namespace Moonshot;
using namespace Moonshot::Test;
using namespace Moonshot::Test::TestUtilities;

ExprTests::ExprTests()
{
}

ExprTests::~ExprTests()
{
}

std::string ExprTests::getTestName() const
{
	return "Expressions Main (no variables)";
}

bool ExprTests::runTest(Context & context)
{
	// read files
	auto correct_test = readFileToVec(context, "res\\tests\\expr\\expr_correct.fox");
	auto bad_test = readFileToVec(context, "res\\tests\\expr\\expr_bad.fox");
	FAILED_RETURN_IF_ERR__SILENT;
	// RUN CORRECT TESTS
	std::cout << std::endl << "Part 1 : Correct tests :\n";
	for (auto& elem : correct_test)
	{

		context.clearLogs(); // clear logs of all the clutter
		std::cout << "\t\xAF Expression :" << elem << std::endl;
		Lexer l(context);
		l.lexStr(elem);
		FAILED_RETURN_IF_ERR("lexing");

		 Parser p(context, l.getTokenVector());
		auto root = p.parseExpr();
		FAILED_RETURN_IF_ERR("parsing (parsing error)");
		FAILED_RETURN_IF(!root, "parsing (null node)")

		TypeCheckVisitor tc(context, true);
		root->accept(tc);
		FAILED_RETURN_IF_ERR("typechecking");

		if (context.optionsManager_.getAttr(OptionsList::exprtest_printAST).value_or(false).get<bool>())
		{
			Dumper dmp;
			root->accept(dmp);
		}

		RTExprVisitor evaluator(context);
		root->accept(evaluator);
		auto result = evaluator.getResult();

		FAILED_RETURN_IF_ERR("evaluation");
		std::cout << "\t\t\xC0 Result: " << TypeUtils::dumpFVal(result) << std::endl;
		FAILED_RETURN_IF(!std::holds_alternative<bool>(result), "evaluation (result wasn't of the expected type)");
		FAILED_RETURN_IF(!std::get<bool>(result), "evaluation (result was false)");
	}
	// RUN INCORRECT TESTS
	std::cout << std::endl << "Part 2 : Incorrect tests :\n";
	for (auto& elem : bad_test)
	{
		context.clearLogs();
		context.resetState();
		std::cout << "\t\xAF Expression :" << elem << std::endl;
		Lexer l(context);
		l.lexStr(elem);
		SUCCESS_CONTINUE_IF_ERR;

		Parser p(context, l.getTokenVector());
		auto root = p.parseExpr();
		SUCCESS_CONTINUE_IF_ERR;
		SUCCESS_CONTINUE_IF(!root);

		TypeCheckVisitor tc(context, true);
		root->accept(tc);
		SUCCESS_CONTINUE_IF_ERR;

		RTExprVisitor evaluator(context);
		root->accept(evaluator);
		auto result = evaluator.getResult();
		SUCCESS_CONTINUE_IF_ERR;

		if (context.isSafe())
		{
			std::cout << "\t\tTest failed (The test was meant to fail, and completed successfully.)\n";
			return false;
		}
	}
	context.resetState();
	return true;
}
