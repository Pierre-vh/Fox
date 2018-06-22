////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ParserTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//	(Unit) Tests for the Parser.
////------------------------------------------------------////

#include "gtest/gtest.h"
#include "Support/TestUtils.hpp"

#include "Moonshot/Fox/Lexer/Lexer.hpp"
#include "Moonshot/Fox/Parser/Parser.hpp"
#include "Moonshot/Fox/Common/Context.hpp"
#include "Moonshot/Fox/AST/ASTContext.hpp"

#include <vector>
#include <string>
#include <functional>

using namespace Moonshot;
using namespace Moonshot::Tests;

class ParsingFunctionTester
{
	public:
		enum class ReadMode
		{
			INDIVIDUAL_LINES, WHOLE_FILE
		};
		ParsingFunctionTester(std::function<bool(Parser&)> fn) : fn_(fn)
		{
		
		}
		void clearInputs()
		{
			inputs_.clear();
		}
		bool openFile(const ReadMode& rm,const std::string& fp)
		{
			curFilePath_ = fp;
			if (rm == ReadMode::WHOLE_FILE)
			{
				std::string str;
				bool flag = readFileToString(fp, str);
				inputs_.push_back(str);
				return flag;
			}
			else
				return readFileToVec(fp, inputs_);
		}
		bool runTest(const bool& shouldFail = false,const bool& hashtagCommentsEnabled = true)
		{
			failMessage_ = "";
			bool flag = true;
			unsigned int linecount = 0;
			for (const auto& sample : inputs_)
			{
				linecount++;
				if (hashtagCommentsEnabled && (sample[0] == '#'))
					continue;
				Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
				ASTContext astctxt;
				Lexer lex(ctxt, astctxt);
				lex.lexStr(sample);
				Parser parse(ctxt, astctxt, lex.getTokenVector());

				// Important : Make the parser aware that we're running a test.
				// This disables some assertions that are not needed to be verified when running a test
				// that calls individual parsing functions.
				parse.enableTestMode();

				if (fn_(parse))
				{
					if (ctxt.isSafe())
					{
						// Parsing ok
						if (shouldFail)
						{
							// it was not expected
							std::stringstream ss;
							ss << "\n(file " << curFilePath_ << " line " << linecount << ") Test ran successfully, but was expected to fail !";
							failMessage_ += ss.str();
							flag = false;
						}
						continue;
					}
					else
					{
						// Failure somewhere
						// if it was expected, just continue
						if (shouldFail)		
							continue;
						// if it was not expected,
						std::stringstream ss;
						ss << "\n(file " << curFilePath_ << " line " << linecount << ") Parsing function returned a valid result, but the context was not safe ?\n";
						failMessage_ += ss.str();
						failMessage_ += ctxt.getLogs();
						flag = false;
					}
				}
				else
				{
					// Failure somewhere in the parsing
					if (!shouldFail)
					{
						std::stringstream ss;
						ss << "\n(file " << curFilePath_ << " line " << linecount << ") Parsing function returned a invalid result, or reported errors.\n";
						failMessage_ += ss.str();
						failMessage_ += ctxt.getLogs();
						flag = false;
					}
					continue;
				}
			}
			return flag;
		}
		std::string getLatestFailureMessage() const
		{
			return failMessage_;
		}
	private:
		std::function<bool(Parser&)> fn_;
		std::string failMessage_,curFilePath_;
		std::vector<std::string> inputs_;
};

TEST(ParserTests,Expressions)
{
	// filepaths
	std::string corr_path = "parser/inputs/expr/correct.fox";
	std::string bad_path = "parser/inputs/expr/incorrect.fox";
	ParsingFunctionTester tester([&](Parser & parse) -> bool {
		auto res = parse.parseExpr();
		return res.isUsable();
	});
	// Correct inputs
	ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::INDIVIDUAL_LINES, corr_path)) << "Could not open file \"" << corr_path << '"';
	EXPECT_TRUE(tester.runTest()) << tester.getLatestFailureMessage();
	tester.clearInputs();
	// Incorrect inputs
	ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::WHOLE_FILE, bad_path)) << "Could not open file \"" << bad_path << '"';
	EXPECT_TRUE(tester.runTest(true)) << tester.getLatestFailureMessage();
}


TEST(ParserTests,ExpressionsStmt)
{
	// Correct inputs
	std::string corr_path = "parser/inputs/exprstmt/correct.fox";
	std::string bad_path = "parser/inputs/exprstmt/incorrect.fox";
	ParsingFunctionTester tester([&](Parser & parse) -> bool {
		auto res = parse.parseExprStmt();
		return res.isUsable();
	});
	// Correct inputs
	ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::INDIVIDUAL_LINES, corr_path)) << "Could not open file \"" << corr_path << '"';
	EXPECT_TRUE(tester.runTest()) << tester.getLatestFailureMessage();
	tester.clearInputs();
	// Incorrect inputs
	ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::WHOLE_FILE, bad_path)) << "Could not open file \"" << bad_path << '"';
	EXPECT_TRUE(tester.runTest(true)) << tester.getLatestFailureMessage();
}

TEST(ParserTests, DeclRef)
{
	std::string corr_path = "parser/inputs/declcall/correct.fox";
	ParsingFunctionTester tester([&](Parser & parse) -> bool {
		auto res = parse.parseDeclRef();
		return res.isUsable();
	});
	ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::INDIVIDUAL_LINES, corr_path)) << "Could not open file \"" << corr_path << '"';
	EXPECT_TRUE(tester.runTest()) << tester.getLatestFailureMessage();
}

TEST(ParserTests, VarDecls)
{
	// Correct inputs
	std::string corr_path = "parser/inputs/var_decls/correct.fox";
	std::string bad_path = "parser/inputs/var_decls/incorrect.fox";
	ParsingFunctionTester tester([&](Parser & parse) -> bool {
		auto res = parse.parseVarDecl();
		return res.isUsable();
	});
	// Correct inputs
	ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::INDIVIDUAL_LINES, corr_path)) << "Could not open file \"" << corr_path << '"';
	EXPECT_TRUE(tester.runTest()) << tester.getLatestFailureMessage();
	tester.clearInputs();
	// Incorrect inputs
	ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::WHOLE_FILE, bad_path)) << "Could not open file \"" << bad_path << '"';
	EXPECT_TRUE(tester.runTest(true)) << tester.getLatestFailureMessage();
}

TEST(ParserTests, FuncDecl)
{
	std::string corr_base_path = "parser/inputs/funcdecl/correct_";
	std::string bad_base_path = "parser/inputs/funcdecl/incorrect_";
	ParsingFunctionTester tester([&](Parser & parse) -> bool {
		auto res = parse.parseFunctionDecl();
		return res.isUsable();
	});
	std::stringstream ss;
	for (int k = 1; k <= 6; k++)
	{
		ss.str(std::string());
		ss << corr_base_path << k << ".fox";
		std::string cur_corr_path = ss.str();
		ss.str(std::string());
		ss << bad_base_path << k << ".fox";
		std::string cur_bad_path = ss.str();

		// correct test
		tester.clearInputs();
		ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::WHOLE_FILE, cur_corr_path)) << "Could not open file \"" << cur_corr_path << '"';
		EXPECT_TRUE(tester.runTest()) << tester.getLatestFailureMessage();
		// bad test
		tester.clearInputs();
		ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::WHOLE_FILE, cur_bad_path)) << "Could not open file \"" << cur_bad_path << '"';
		EXPECT_TRUE(tester.runTest(true)) << tester.getLatestFailureMessage();
	}
}

TEST(ParserTests, Conditions)
{
	std::string corr_base_path = "parser/inputs/condition/correct_";
	std::string bad_base_path = "parser/inputs/condition/incorrect_";
	ParsingFunctionTester tester([&](Parser & parse) -> bool {
		auto res = parse.parseCondition();
		return res.isUsable();
	});
	std::stringstream ss;
	for (int k = 1; k <= 5; k++)
	{
		ss.str(std::string());
		ss << corr_base_path << k << ".fox";
		std::string cur_corr_path = ss.str();
		ss.str(std::string());
		ss << bad_base_path << k << ".fox";
		std::string cur_bad_path = ss.str();

		// correct test
		tester.clearInputs();
		ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::WHOLE_FILE, cur_corr_path)) << "Could not open file \"" << cur_corr_path << '"';
		EXPECT_TRUE(tester.runTest()) << tester.getLatestFailureMessage();
		// bad test
		tester.clearInputs();
		ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::WHOLE_FILE, cur_bad_path)) << "Could not open file \"" << cur_bad_path << '"';
		EXPECT_TRUE(tester.runTest(true)) << tester.getLatestFailureMessage();
	}
}

TEST(ParserTests, WhileLoops)
{
	std::string corr_base_path = "parser/inputs/whileloop/correct_";
	std::string bad_base_path = "parser/inputs/whileloop/incorrect_";
	ParsingFunctionTester tester([&](Parser & parse) -> bool {
		auto res = parse.parseWhileLoop();
		return res.isUsable();
	});
	std::stringstream ss;
	for (int k = 1; k <= 4; k++)
	{
		ss.str(std::string());
		ss << corr_base_path << k << ".fox";
		std::string cur_corr_path = ss.str();
		ss.str(std::string());
		ss << bad_base_path << k << ".fox";
		std::string cur_bad_path = ss.str();

		// correct test
		tester.clearInputs();
		ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::WHOLE_FILE, cur_corr_path)) << "Could not open file \"" << cur_corr_path << '"';
		EXPECT_TRUE(tester.runTest()) << tester.getLatestFailureMessage();
		// bad test
		tester.clearInputs();
		ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::WHOLE_FILE, cur_bad_path)) << "Could not open file \"" << cur_bad_path << '"';
		EXPECT_TRUE(tester.runTest(true)) << tester.getLatestFailureMessage();
	}
}

TEST(ParserTests, CompoundStmts)
{
	std::string corr_base_path = "parser/inputs/compoundstmt/correct_";
	std::string bad_base_path = "parser/inputs/compoundstmt/incorrect_";
	ParsingFunctionTester tester([&](Parser & parse) -> bool {
		auto res = parse.parseCompoundStatement();
		return res.isUsable();
	});
	std::stringstream ss;
	for (int k = 1; k <= 2; k++)
	{
		ss.str(std::string());
		ss << corr_base_path << k << ".fox";
		std::string cur_corr_path = ss.str();
		ss.str(std::string());
		ss << bad_base_path << k << ".fox";
		std::string cur_bad_path = ss.str();

		// correct test
		tester.clearInputs();
		ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::WHOLE_FILE, cur_corr_path)) << "Could not open file \"" << cur_corr_path << '"';
		EXPECT_TRUE(tester.runTest()) << tester.getLatestFailureMessage();
		// bad test
		tester.clearInputs();
		ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::WHOLE_FILE, cur_bad_path)) << "Could not open file \"" << cur_bad_path << '"';
		EXPECT_TRUE(tester.runTest(true)) << tester.getLatestFailureMessage();
	}
}

TEST(ParserTests, Unit)
{
	std::string corr_base_path = "parser/inputs/unit/correct_";
	std::string bad_base_path = "parser/inputs/unit/incorrect_";


	ParsingFunctionTester tester([&](Parser & parse) -> bool {
		
		auto res = parse.parseUnit(
			FileID(), // No need for a valid FileID in a test.
			parse.getASTContext().identifiers.getUniqueIdentifierInfo("Test Unit")
		);
		return res;
	});

	std::stringstream ss;
	for (int k = 1; k <= 3; k++)
	{
		ss.str(std::string());
		ss << corr_base_path << k << ".fox";
		std::string cur_corr_path = ss.str();
		ss.str(std::string());
		ss << bad_base_path << k << ".fox";
		std::string cur_bad_path = ss.str();

		// correct test
		tester.clearInputs();
		ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::WHOLE_FILE, cur_corr_path)) << "Could not open file \"" << cur_corr_path << '"';
		EXPECT_TRUE(tester.runTest()) << tester.getLatestFailureMessage();
		// bad test
		tester.clearInputs();
		ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::WHOLE_FILE, cur_bad_path)) << "Could not open file \"" << cur_bad_path << '"';
		EXPECT_TRUE(tester.runTest(true)) << tester.getLatestFailureMessage();
	}
}