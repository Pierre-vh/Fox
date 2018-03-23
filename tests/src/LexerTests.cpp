#include "gtest/gtest.h"
#include "TestUtils/TestUtils.hpp"

#include "Moonshot/Fox/Lexer/Lexer.hpp"
#include "Moonshot/Common/Context/Context.hpp"

using namespace Moonshot;
using namespace Moonshot::Tests;

TEST(LexerTests,CorrectTest1)
{
	std::string file_content, file_path;
	file_path = "lexer/correct_1.fox";
	ASSERT_TRUE(readFileToString(file_path,file_content)) << "Could not open test file \"" << file_path << '"';

	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	Lexer lex(ctxt);
	lex.lexStr(file_content);
	ASSERT_TRUE(ctxt.isSafe()) << "Context reported one or more errors while lexing the file. Context log:\n" << ctxt.getLogs();
}

TEST(LexerTests, IncorrectTest1)
{
	std::string file_content, file_path;
	file_path = "lexer/incorrect_1.fox";
	ASSERT_TRUE(readFileToString(file_path, file_content)) << "Could not open test file \"" << file_path << '"';

	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	Lexer lex(ctxt);
	lex.lexStr(file_content);
	ASSERT_FALSE(ctxt.isSafe()) << "Test completed successfully, but was expected to fail.";
}

TEST(LexerTests, IncorrectTest2)
{
	std::string file_content, file_path;
	file_path = "lexer/incorrect_2.fox";
	ASSERT_TRUE(readFileToString(file_path, file_content)) << "Could not open test file \"" << file_path << '"';

	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	Lexer lex(ctxt);
	lex.lexStr(file_content);
	ASSERT_FALSE(ctxt.isSafe()) << "Test completed successfully, but was expected to fail.";
}

TEST(LexerTests, IncorrectTest3)
{
	std::string file_content, file_path;
	file_path = "lexer/incorrect_3.fox";
	ASSERT_TRUE(readFileToString(file_path, file_content)) << "Could not open test file \"" << file_path << '"';

	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	Lexer lex(ctxt);
	lex.lexStr(file_content);
	ASSERT_FALSE(ctxt.isSafe()) << "Test completed successfully, but was expected to fail.";
}

TEST(LexerTests, IncorrectTest4)
{
	std::string file_content, file_path;
	file_path = "lexer/incorrect_3.fox";
	ASSERT_TRUE(readFileToString(file_path, file_content)) << "Could not open test file \"" << file_path << '"';

	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	Lexer lex(ctxt);
	lex.lexStr(file_content);
	ASSERT_FALSE(ctxt.isSafe()) << "Test completed successfully, but was expected to fail.";
}