////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Conditions.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Conditions.hpp"
#include "Moonshot/Fox/AST/Dumper/Dumper.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTStmt.hpp"

using namespace Moonshot;
using namespace Moonshot::Test;
using namespace Moonshot::Test::TestUtilities;

std::string Conditions::getTestName() const
{
	return "Conditions (if/else if/else)";
}

bool Conditions::runTest(Context & context)
{
	std::vector<std::string> good_tests_fp =
	{
		"res\\tests\\conditions\\correct\\condition_1.fox",
		"res\\tests\\conditions\\correct\\condition_2.fox",
		"res\\tests\\conditions\\correct\\condition_3.fox",
		"res\\tests\\conditions\\correct\\condition_4.fox",
		"res\\tests\\conditions\\correct\\condition_5.fox"
	};
	std::vector<std::string> bad_tests_fp =
	{
		"res\\tests\\conditions\\bad\\condition_1.fox",
		"res\\tests\\conditions\\bad\\condition_2.fox",
		"res\\tests\\conditions\\bad\\condition_3.fox",
		"res\\tests\\conditions\\bad\\condition_4.fox",
		"res\\tests\\conditions\\bad\\condition_5.fox"
	};
	// good tests
	int goodcount = 0;
	for (const auto& elem : good_tests_fp)
	{
		goodcount++;
		auto data = readFileToString(context,elem);
		FAILED_RETURN_IF_ERR__SILENT;
		FAILED_RETURN_IF(!testCond(context, data),elem);
		std::cout << "\t\t\xC0 (Correct Test No " << goodcount << ") Success\n"; 
	}
	// bad tests
	int badcount = 0;
	for (const auto& elem : bad_tests_fp)
	{
		badcount++;
		context.clearLogs();
		context.resetState();
		auto data = readFileToString(context, elem);
		FAILED_RETURN_IF_ERR__SILENT;
		if (testCond(context, data))
		{
			std::cout << "\t\t\xC0 (Bad Test No " << badcount << ") Failure (Test was successful, but should have failed.)\n";
			return false;
		}
		else 
		{ 
			std::cout << "\t\t\xC0 (Bad Test No " << badcount << ") Success (Test Failed as Expected.)\n"; 
			continue; 
		}
	}
	return true;
}

bool Conditions::testCond(Context & context, const std::string& str)
{
	context.clearLogs();
	context.resetState();
	Lexer lex(context);
	lex.lexStr(str);
	FAILED_RETURN_IF_ERR__SILENT;
	Parser parser(context, lex.getTokenVector());

	std::unique_ptr<IASTStmt> node;
	if (auto parseres = parser.parseCondition())
		node = std::move(parseres.result_);

	context.printLogs();
	FAILED_RETURN_IF_ERR__SILENT;
	if (!node) return false;
	node->accept(Dumper());
	return true;
}