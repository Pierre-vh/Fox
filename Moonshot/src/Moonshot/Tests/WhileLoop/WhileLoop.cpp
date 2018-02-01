////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : WhileLoop.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "WhileLoop.h"

using namespace Moonshot;
using namespace Moonshot::TestUtilities;

WhileLoop::~WhileLoop()
{
}

std::string WhileLoop::getTestName() const
{
	return "While Loops";
}

bool WhileLoop::runTest(Context & context)
{
	std::vector<std::string> good_tests_fp =
	{
		"res\\tests\\whileloop\\correct\\while_loop_1.fox",
		"res\\tests\\whileloop\\correct\\while_loop_2.fox",
	};
	std::vector<std::string> bad_tests_fp =
	{
		"res\\tests\\whileloop\\bad\\while_loop_1.fox",
		"res\\tests\\whileloop\\bad\\while_loop_2.fox",
		"res\\tests\\whileloop\\bad\\while_loop_3.fox",
	};
	// good tests
	int goodcount = 0;
	for (auto& elem : good_tests_fp)
	{
		goodcount++;
		auto data = readFileToString(context, elem);
		FAILED_RETURN_IF_ERR__SILENT;
		FAILED_RETURN_IF(!testWhileLoop(context, data), elem);
		std::cout << "\t\t\xC0 (Correct Test No " << goodcount << ") Success\n";
	}
	// bad tests
	int badcount = 0;
	for (auto& elem : bad_tests_fp)
	{
		badcount++;
		context.clearLogs();
		context.resetState();
		auto data = readFileToString(context, elem);
		FAILED_RETURN_IF_ERR__SILENT;
		if (testWhileLoop(context, data))
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

bool WhileLoop::testWhileLoop(Context & context, const std::string & str)
{
	context.clearLogs();
	context.resetState();
	Lexer lex(context);
	lex.lexStr(str);
	FAILED_RETURN_IF_ERR__SILENT;
	Parser parser(context, lex.getTokenVector());
	auto node = parser.parseWhileLoop();
	FAILED_RETURN_IF_ERR__SILENT;
	if (!node) return false;
	node->accept(Dumper());
	return true;
}
