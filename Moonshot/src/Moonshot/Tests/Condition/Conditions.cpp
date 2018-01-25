////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Conditions.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Conditions.h"

using namespace Moonshot;
using namespace Moonshot::TestUtilities;

Conditions::Conditions()
{
}


Conditions::~Conditions()
{
}

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
	for (auto& elem : good_tests_fp)
	{
		goodcount++;
		auto data = readFileToString(context,elem);
		FAILED_RETURN_IF_ERR__SILENT;
		FAILED_RETURN_IF(!testCondFile(context, data),elem);
		std::cout << "\t\t\xC0 (Correct Test No " << goodcount << ") Success" << std::endl; 
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
		if (testCondFile(context, data))
		{
			std::cout << "\t\t\xC0 (Bad Test No " << badcount << ") Failure (Test was successful, but should have failed.)" << std::endl;
			return false;
		}
		FAILED_RETURN_IF(testCondFile(context, data), elem);
		if (!context.isSafe()) 
		{ 
			std::cout << "\t\t\xC0 (Bad Test No " << badcount << ") Success (Test Failed as Expected.)" << std::endl; 
			continue; 
		}
	}
	return true;
}

bool Conditions::testCondFile(Context & context, const std::string& str)
{
	context.clearLogs();
	context.resetState();
	Lexer lex(context);
	lex.lexStr(str);
	FAILED_RETURN_IF_ERR__SILENT;
	Parser parser(context, lex.getTokenVector());
	auto node = parser.parseCondition();
	FAILED_RETURN_IF_ERR__SILENT;
	if (!node) return false;
	node->accept(Dumper());
	return true;
}