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
	RETURN_SILENTLY_IF_ERR;
	// RUN CORRECT TESTS
	for (auto& elem : correct_test)
	{
		Lexer l(context);
		l.lexStr(elem);
		RETURN_IF_ERR("lexing");

		Parser p(context, l);
		auto root = p.parseExpr();
		RETURN_IF_ERR("parsing");

		root->accept(TypeCheckVisitor(context,true));
		RETURN_IF_ERR("typechecking");

		auto result = root->accept(RTExprVisitor(context));
		RETURN_IF_ERR("evaluation");

		std::cout << "\tExpression :" << elem << std::endl;
		std::cout << "\t\tResult: " << fv_util::dumpFVal(result) << std::endl;
	}
	std::cout << std::endl;
	// todo : add bad tests
	return true;
}
