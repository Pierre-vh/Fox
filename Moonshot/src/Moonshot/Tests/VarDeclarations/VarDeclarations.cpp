////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : VarDeclarations.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "VarDeclarations.h"

using namespace Moonshot;
using namespace Moonshot::TestUtilities;

VarDeclarations::~VarDeclarations()
{

}

std::string VarDeclarations::getTestName() const
{
	return "Variables Declarations";
}

bool VarDeclarations::runTest(Context & context)
{
	// read files
	auto correct_test = readFileToVec(context, "res\\tests\\vdecl\\vdecl_correct.fox");
	auto bad_test = readFileToVec(context, "res\\tests\\vdecl\\vdecl_bad.fox");
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
		auto root = p.parseVarDeclStmt();
		FAILED_RETURN_IF_ERR("parsing");

		root->accept(TypeCheckVisitor(context, true));
		FAILED_RETURN_IF_ERR("typechecking");

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
		auto root = p.parseVarDeclStmt();

		SUCCESS_CONTINUE_IF_ERR;
		SUCCESS_CONTINUE_IF(!root); // fail if root's false

		root->accept(TypeCheckVisitor(context, true));
		SUCCESS_CONTINUE_IF_ERR;

		if (context.isSafe())
		{
			std::cout << "\t\tTest failed (The test was meant to fail, but completed successfully.)\n";
			return false;
		}
	}
	context.resetState();
	return true;
}
