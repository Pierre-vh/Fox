////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ExprStmt.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ExprStmt.hpp"
#include "../../Common/Types/TypesUtils.hpp"
using namespace Moonshot;
using namespace Moonshot::TestUtilities;

ExprStmtTest::ExprStmtTest()
{
}


ExprStmtTest::~ExprStmtTest()
{
}

std::string ExprStmtTest::getTestName() const
{
	return "Expression Statements Tests.";
}

bool ExprStmtTest::runTest(Context & context)
{
	// read files
	auto correct_test = readFileToVec(context, "res\\tests\\exprstmt\\exprstmt_correct.fox");
	auto bad_test = readFileToVec(context, "res\\tests\\exprstmt\\exprstmt_bad.fox");
	FAILED_RETURN_IF_ERR__SILENT;
	// RUN CORRECT TESTS
	std::cout << std::endl << "Part 1 : Correct tests :\n";
	for (auto& elem : correct_test)
	{
		std::cout << "\t\xAF Expression :" << elem << std::endl;
		Lexer l(context);
		l.lexStr(elem);
		FAILED_RETURN_IF_ERR("lexing");

		 Parser p(context, l.getTokenVector());
		auto root = p.parseExprStmt();
		FAILED_RETURN_IF_ERR("parsing");

		root->accept(TypeCheckVisitor(context, true));
		FAILED_RETURN_IF_ERR("typechecking");

		RTExprVisitor evaluator(context);
		root->accept(evaluator);
		auto result = evaluator.getResult();
		FAILED_RETURN_IF_ERR("evaluation");
		std::cout << "\t\t\xC0 Result: " << fv_util::dumpFVal(result) << std::endl;
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
		auto root = p.parseExprStmt();

		SUCCESS_CONTINUE_IF_ERR;
		SUCCESS_CONTINUE_IF(!root); // fail if root's false

		root->accept(TypeCheckVisitor(context, true));
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