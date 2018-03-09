////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : FuncDecl.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "FuncDecl.hpp"

#include "Moonshot/Fox/AST/Nodes/ASTDecl.hpp"
#include "Moonshot/Fox/AST/Dumper/Dumper.hpp"

using namespace Moonshot;
using namespace Moonshot::Test;
using namespace Moonshot::Test::TestUtilities;

FuncDecl::~FuncDecl()
{
}

std::string FuncDecl::getTestName() const
{
	return "Function Declarations";
}

bool FuncDecl::runTest(Context & context)
{
	std::vector<std::string> good_tests_fp =
	{
		"res\\tests\\funcdecl\\correct\\funcdecl_1.fox",
		"res\\tests\\funcdecl\\correct\\funcdecl_2.fox",
		"res\\tests\\funcdecl\\correct\\funcdecl_3.fox",
		"res\\tests\\funcdecl\\correct\\funcdecl_4.fox",
		"res\\tests\\funcdecl\\correct\\funcdecl_5.fox",
		"res\\tests\\funcdecl\\correct\\funcdecl_6.fox",
	};
	std::vector<std::string> bad_tests_fp =
	{
		"res\\tests\\funcdecl\\bad\\funcdecl_1.fox",
		"res\\tests\\funcdecl\\bad\\funcdecl_2.fox",
		"res\\tests\\funcdecl\\bad\\funcdecl_3.fox",
		"res\\tests\\funcdecl\\bad\\funcdecl_4.fox",
		"res\\tests\\funcdecl\\bad\\funcdecl_5.fox",
		"res\\tests\\funcdecl\\bad\\funcdecl_6.fox",
	};
	// good tests
	int goodcount = 0;
	for (const auto& elem : good_tests_fp)
	{
		goodcount++;
		auto data = readFileToString(context, elem);
		FAILED_RETURN_IF_ERR__SILENT;
		FAILED_RETURN_IF(!testFuncDecl(context, data), elem);
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
		if (testFuncDecl(context, data))
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

bool FuncDecl::testFuncDecl(Context & context, const std::string & str)
{
	context.clearLogs();
	context.resetState();
	Lexer lex(context);
	lex.lexStr(str);
	FAILED_RETURN_IF_ERR__SILENT;
	Parser parser(context, lex.getTokenVector());

	std::unique_ptr<ASTFunctionDecl> node;
	if (auto parseres = parser.parseFunctionDeclaration())
		node = std::move(parseres.result_);

	context.printLogs();
	FAILED_RETURN_IF_ERR__SILENT;
	if (!node) return false;
	node->accept(Dumper());
	return true;
}
