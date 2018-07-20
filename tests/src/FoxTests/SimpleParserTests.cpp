////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : SimpleParserTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//	Simple Tests for the Parser. They test that the parser
//  recognize an input correctly, and return a valid result.
////------------------------------------------------------////

#include "gtest/gtest.h"
#include "Support/TestUtils.hpp"

#include "Fox/Lexer/Lexer.hpp"
#include "Fox/Parser/Parser.hpp"
#include "Fox/AST/ASTContext.hpp"

#include <vector>
#include <string>
#include <functional>

using namespace fox;
using namespace fox::test;

class ParsingFunctionTester
{
	public:
		enum class ReadMode
		{
			INDIVIDUAL_LINES, WHOLE_FILE
		};

		ParsingFunctionTester(std::function<bool(const FileID&,Parser&)> fn) : fn_(fn)
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
				SourceManager srcMgr;
				DiagnosticEngine dg(srcMgr);
				ASTContext astctxt;
				DeclContext dc;
				Lexer lex(dg, srcMgr, astctxt);
				auto file = srcMgr.loadFromString(sample);

				if (!file)
				{
					failMessage_ = "Failed to load string in SourceManager";
					return false;
				}

				lex.lexFile(file);

				if (dg.getErrorsCount())
				{
					failMessage_ = "Failed at lexing of sentence \"" + sample + "\"";
					flag = false;
					continue;
				}

				Parser parse(dg, srcMgr, astctxt, lex.getTokenVector(), &dc);

				if (fn_(file,parse))
				{
					if (dg.getErrorsCount() == 0)
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
						ss << "\n(file " << curFilePath_ << " line " << linecount << ") Parsing function returned a valid result, but errors occured.\n";
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
		std::function<bool(const FileID&,Parser&)> fn_;
		std::string failMessage_,curFilePath_;
		std::vector<std::string> inputs_;
};

TEST(SimpleParserTests,Expressions)
{
	// filepaths
	std::string corr_path = "parser/simple/expr/correct.fox";
	std::string bad_path = "parser/simple/expr/incorrect.fox";
	ParsingFunctionTester tester([&](const FileID& file, Parser & parse) -> bool {
		auto res = parse.parseExpr();
		return res.isUsable();
	});
	// Correct inputs
	ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::INDIVIDUAL_LINES, corr_path)) << "Could not open file \"" << corr_path << '"';
	EXPECT_TRUE(tester.runTest()) << tester.getLatestFailureMessage();
	tester.clearInputs();
	// Incorrect inputs
	ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::INDIVIDUAL_LINES, bad_path)) << "Could not open file \"" << bad_path << '"';
	EXPECT_TRUE(tester.runTest(true)) << tester.getLatestFailureMessage();
}


TEST(SimpleParserTests,ExpressionsStmt)
{
	// Correct inputs
	std::string corr_path = "parser/simple/exprstmt/correct.fox";
	std::string bad_path = "parser/simple/exprstmt/incorrect.fox";
	ParsingFunctionTester tester([&](const FileID& file, Parser & parse) -> bool {
		auto res = parse.parseExprStmt();
		return res.isUsable();
	});
	// Correct inputs
	ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::INDIVIDUAL_LINES, corr_path)) << "Could not open file \"" << corr_path << '"';
	EXPECT_TRUE(tester.runTest()) << tester.getLatestFailureMessage();
	tester.clearInputs();
	// Incorrect inputs
	ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::INDIVIDUAL_LINES, bad_path)) << "Could not open file \"" << bad_path << '"';
	EXPECT_TRUE(tester.runTest(true)) << tester.getLatestFailureMessage();
}

TEST(SimpleParserTests, DeclRef)
{
	std::string corr_path = "parser/simple/declcall/correct.fox";
	ParsingFunctionTester tester([&](const FileID& file, Parser & parse) -> bool {
		auto res = parse.parseDeclRef();
		return res.isUsable();
	});
	ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::INDIVIDUAL_LINES, corr_path)) << "Could not open file \"" << corr_path << '"';
	EXPECT_TRUE(tester.runTest()) << tester.getLatestFailureMessage();
}

TEST(SimpleParserTests, VarDecls)
{
	// Correct inputs
	std::string corr_path = "parser/simple/var_decls/correct.fox";
	std::string bad_path = "parser/simple/var_decls/incorrect.fox";
	ParsingFunctionTester tester([&](const FileID& file, Parser & parse) -> bool {
		auto res = parse.parseVarDecl();
		return res.isUsable();
	});
	// Correct inputs
	ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::INDIVIDUAL_LINES, corr_path)) << "Could not open file \"" << corr_path << '"';
	EXPECT_TRUE(tester.runTest()) << tester.getLatestFailureMessage();
	tester.clearInputs();
	// Incorrect inputs
	ASSERT_TRUE(tester.openFile(ParsingFunctionTester::ReadMode::INDIVIDUAL_LINES, bad_path)) << "Could not open file \"" << bad_path << '"';
	EXPECT_TRUE(tester.runTest(true)) << tester.getLatestFailureMessage();
}

TEST(SimpleParserTests, FuncDecl)
{
	std::string corr_base_path = "parser/simple/funcdecl/correct_";
	std::string bad_base_path = "parser/simple/funcdecl/incorrect_";
	ParsingFunctionTester tester([&](const FileID& file, Parser & parse) -> bool {
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

TEST(SimpleParserTests, Conditions)
{
	std::string corr_base_path = "parser/simple/condition/correct_";
	std::string bad_base_path = "parser/simple/condition/incorrect_";
	ParsingFunctionTester tester([&](const FileID& file, Parser & parse) -> bool {
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

TEST(SimpleParserTests, WhileLoops)
{
	std::string corr_base_path = "parser/simple/whileloop/correct_";
	std::string bad_base_path = "parser/simple/whileloop/incorrect_";
	ParsingFunctionTester tester([&](const FileID& file, Parser & parse) -> bool {
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

TEST(SimpleParserTests, CompoundStmts)
{
	std::string corr_base_path = "parser/simple/compoundstmt/correct_";
	std::string bad_base_path = "parser/simple/compoundstmt/incorrect_";
	ParsingFunctionTester tester([&](const FileID& file, Parser & parse) -> bool {
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

TEST(SimpleParserTests, Unit)
{
	std::string corr_base_path = "parser/simple/unit/correct_";
	std::string bad_base_path = "parser/simple/unit/incorrect_";


	ParsingFunctionTester tester([&](const FileID& file, Parser & parse) -> bool {
		
		auto res = parse.parseUnit(
			file,
			parse.getASTContext().identifiers.getUniqueIdentifierInfo("Test Unit"),
			/* is main unit */ true
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