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
#include <functional>

using namespace Moonshot;
using namespace Moonshot::Tests;

class ParsingFunctionTester
{
	public:
		ParsingFunctionTester(std::function<bool(Parser&)> fn) : fn_(fn)
		{
		
		}
		bool runTest(const std::string& sample,const bool& shouldFail = false)
		{
			failMessage_ = "";
			Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
			Lexer lex(ctxt);
			lex.lexStr(sample);
			Parser parse(ctxt, lex.getTokenVector());
			if (fn_(parse))
			{
				// Parsing went correctly, check context then return
				if (ctxt.isSafe())
					return !shouldFail;
				else
				{
					failMessage_ += "Parsing function returned a valid result, but the context was not safe ?";
					failMessage_ += ctxt.getLogs();
					return shouldFail;
				}
			}
			else
			{
				if (!shouldFail)
				{
					failMessage_ += "Parsing function returned a invalid result, or reported errors.";
					failMessage_ += ctxt.getLogs();
				}
				return shouldFail;
			}
		}
		std::string getFailureMessage() const
		{
			return failMessage_;
		}
	private:
		// test function : returns true/false to signal that the parsing went correctly
		std::function<bool(Parser&)> fn_;
		// the latest failure message.
		std::string failMessage_;
};

// Find a way to make theses tests generic? The code is pretty much the same for the 3 functions except for the filenames and parsing function..
// Idea : improve the ParsingFunctionTester to open a file and read it as a whole or line by line and run the test on each line.
// syntax idea:
// ASSERT_TRUE(tester.openFile()) << "Could not open file " << tester.filename();
// EXPECT_TRUE(tester.runTest())

TEST(ParserTests,Expressions)
{
	// Correct inputs
	std::string corr_path = "parser/expr/correct.fox";
	std::vector<std::string> correctInputs;
	ASSERT_TRUE(readFileToVec(corr_path, correctInputs)) << "Could not open test file \"" << corr_path << '"';
	unsigned int linecounter = 0;
	for (const auto& line : correctInputs)
	{
		// ignore lines beginning with a #
		linecounter++;
		if (line[0] == '#')
			continue;

		ParsingFunctionTester tester([&](Parser & parse) -> bool {
			auto res = parse.parseExpr();
			return res && res.result_; // Parsing result is valid & node is not null
		});
		EXPECT_TRUE(tester.runTest(line,false)) << "Expression at line \"" << line << "\" failed.\n" << tester.getFailureMessage();
	}

	std::string bad_path = "parser/expr/incorrect.fox";
	std::vector<std::string> badInputs;
	ASSERT_TRUE(readFileToVec(bad_path, badInputs)) << "Could not open test file \"" << bad_path << '"';
	linecounter = 0;
	for (const auto& line : badInputs)
	{
		// ignore lines beginning with a #
		linecounter++;
		if (line[0] == '#')
			continue;

		ParsingFunctionTester tester([&](Parser & parse) -> bool {
			auto res = parse.parseExpr();
			return res && res.result_; // Parsing result is valid & node is not null
		});
		EXPECT_TRUE(tester.runTest(line, true)) << "Expression at line \"" << line << "\" failed.\n" << tester.getFailureMessage();
	}
}


TEST(ParserTests,ExpressionsStmt)
{
	// Correct inputs
	std::string corr_path = "parser/exprstmt/correct.fox";
	std::vector<std::string> correctInputs;
	ASSERT_TRUE(readFileToVec(corr_path, correctInputs)) << "Could not open test file \"" << corr_path << '"';
	unsigned int linecounter = 0;
	for (const auto& line : correctInputs)
	{
		// ignore lines beginning with a #
		linecounter++;
		if (line[0] == '#')
			continue;

		ParsingFunctionTester tester([&](Parser & parse) -> bool {
			auto res = parse.parseExprStmt();
			return res && res.result_; // Parsing result is valid & node is not null
		});
		EXPECT_TRUE(tester.runTest(line, false)) << "Expression at line \"" << line << "\" failed.\n" << tester.getFailureMessage();
	}

	std::string bad_path = "parser/exprstmt/incorrect.fox";
	std::vector<std::string> badInputs;
	ASSERT_TRUE(readFileToVec(bad_path, badInputs)) << "Could not open test file \"" << bad_path << '"';
	linecounter = 0;
	for (const auto& line : badInputs)
	{
		// ignore lines beginning with a #
		linecounter++;
		if (line[0] == '#')
			continue;

		ParsingFunctionTester tester([&](Parser & parse) -> bool {
			auto res = parse.parseExprStmt();
			return res && res.result_; // Parsing result is valid & node is not null
		});
		EXPECT_TRUE(tester.runTest(line, true)) << "Expression at line \"" << line << "\" failed.\n" << tester.getFailureMessage();
	}
}

TEST(ParserTests, DeclCall)
{
	// Correct inputs
	std::string corr_path = "parser/declcall/correct.fox";
	std::vector<std::string> correctInputs;
	ASSERT_TRUE(readFileToVec(corr_path, correctInputs)) << "Could not open test file \"" << corr_path << '"';
	unsigned int linecounter = 0;
	for (const auto& line : correctInputs)
	{
		// ignore lines beginning with a #
		linecounter++;
		if (line[0] == '#')
			continue;

		ParsingFunctionTester tester([&](Parser & parse) -> bool {
			auto res = parse.parseDeclCall();
			return res && res.result_; // Parsing result is valid & node is not null
		});
		EXPECT_TRUE(tester.runTest(line, false)) << "Expression \"" << line << "\" at line \"" << linecounter << "\" failed.\n" << tester.getFailureMessage();
	}

	std::string bad_path = "parser/declcall/incorrect.fox";
	std::vector<std::string> badInputs;
	ASSERT_TRUE(readFileToVec(bad_path, badInputs)) << "Could not open test file \"" << bad_path << '"';
	linecounter = 0;
	for (const auto& line : badInputs)
	{
		// ignore lines beginning with a #
		linecounter++;
		if (line[0] == '#')
			continue;

		ParsingFunctionTester tester([&](Parser & parse) -> bool {
			auto res = parse.parseDeclCall();
			return res && res.result_; // Parsing result is valid & node is not null
		});
		EXPECT_TRUE(tester.runTest(line, true)) << "Expression \"" << line << "\" at line \"" << linecounter << "\" failed.\n" << tester.getFailureMessage();
	}
}