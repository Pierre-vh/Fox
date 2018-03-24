////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ParserTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//	(Unit) Tests for the Parser.
////------------------------------------------------------////

#include "gtest/gtest.h"
#include "TestUtils/TestUtils.hpp"

#include "Moonshot/Fox/Lexer/Lexer.hpp"
#include "Moonshot/Fox/Parser/Parser.hpp"
#include "Moonshot/Common/Context/Context.hpp"

#include <vector>
#include <string>

using namespace Moonshot;
using namespace Moonshot::Tests;

enum class ReadMode
{
	LINES, WHOLE_FILE
};


TEST(ParserTests,Expressions)
{
	/*
	PARSER_TEST_GROUP("Expressions", expr)
		PARSER_CORRECT_TEST(expr, LINES, "expr/correct.fox", parseExpr())
		PARSER_INCORRECT_TEST(expr, LINES, "expr/incorrect.fox", parseExpr())
	*/
	// Correct inputs
	std::string corr_path = "parser/expr/correct.fox";
	std::vector<std::string> correctInputs;
	ASSERT_TRUE(readFileToVec(corr_path, correctInputs)) << "Could not open test file \"" << corr_path << '"';
	unsigned int linecounter = 0;
	for (const auto& line : correctInputs)
	{
		// ignore lines beginning by a #
		linecounter++;
		if (line[0] == '#')
			continue;
		Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);;
		Lexer l(ctxt);
		l.lexStr(line);
		Parser p(ctxt, l.getTokenVector());
		EXPECT_TRUE(p.parseExpr()) << "The expression \"" << line << "\" at line " << linecounter << " returned a null node";
		EXPECT_TRUE(ctxt.isSafe()) << "Context reported one or more errors while parser the line. Context log:\n" << ctxt.getLogs();
	}

	std::string bad_path = "parser/expr/incorrect.fox";
	std::vector<std::string> badInputs;
	ASSERT_TRUE(readFileToVec(bad_path, badInputs)) << "Could not open test file \"" << bad_path << '"';
	linecounter = 0;
	for (const auto& line : badInputs)
	{
		// ignore lines beginning by a #
		linecounter++;
		if (line[0] == '#')
			continue;

		Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);;
		Lexer l(ctxt);
		l.lexStr(line);
		Parser p(ctxt, l.getTokenVector());
		bool isNodeNull = !p.parseExpr();
		EXPECT_TRUE(isNodeNull || (!ctxt.isSafe())) << "The context was safe and the node returned was not null, which means the parsing was successful (this test expects the parsing to be unsuccessful)";
	}
}