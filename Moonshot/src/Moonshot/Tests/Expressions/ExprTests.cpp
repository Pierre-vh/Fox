////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ExprTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ExprTests.h"

using namespace Moonshot;
using namespace Moonshot::TestUtilities;

ExprTests::ExprTests()
{
}

ExprTests::~ExprTests()
{
}

std::string Moonshot::ExprTests::getTestName() const
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
	std::cout << std::endl << "Part 1 : Correct tests :" << std::endl;
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
		FAILED_RETURN_IF(!root,"parsing (null node)")

		root->accept(TypeCheckVisitor(context,true));
		FAILED_RETURN_IF_ERR("typechecking");

		if (context.options.getAttr(OptionsList::EXPRTEST_print_ast).value_or(false).get<bool>())
			root->accept(Dumper());

		auto result = root->accept(RTExprVisitor(context));
		FAILED_RETURN_IF_ERR("evaluation");
		std::cout << "\t\t\xC0 Result: " << fv_util::dumpFVal(result) << std::endl;
		FAILED_RETURN_IF(!std::holds_alternative<bool>(result), "evaluation (result wasn't of the expected type)");
		FAILED_RETURN_IF(!std::get<bool>(result), "evaluation (result was false)");
	}
	// RUN INCORRECT TESTS
	std::cout << std::endl << "Part 2 : Incorrect tests :" << std::endl;
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

		root->accept(TypeCheckVisitor(context,true));
		SUCCESS_CONTINUE_IF_ERR;

		auto result = root->accept(RTExprVisitor(context));
		SUCCESS_CONTINUE_IF_ERR;

		if (context.isSafe())
		{
			std::cout << "\t\tTest failed (The test was meant to fail, and completed successfully.)" << std::endl;
			return false;
		}
	}
	context.resetState();
	return true;
}
