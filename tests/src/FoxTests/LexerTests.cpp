////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : LexerTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//	(Unit) Tests for the Lexer.
////------------------------------------------------------////

#include "gtest/gtest.h"

#include <memory>

#include "TestUtils/TestUtils.hpp"

#include "Moonshot/Fox/AST/ASTContext.hpp" 
#include "Moonshot/Fox/Lexer/Lexer.hpp"
#include "Moonshot/Fox/Lexer/Token.hpp"
#include "Moonshot/Fox/Common/Context.hpp"

using namespace Moonshot;
using namespace Moonshot::Tests;

TEST(LexerTests,CorrectTest1)
{
	std::string file_content, file_path;
	file_path = "lexer/inputs/correct_1.fox";
	ASSERT_TRUE(readFileToString(file_path,file_content)) << "Could not open test file \"" << file_path << '"';

	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	ASTContext astctxt;
	Lexer lex(ctxt, astctxt);
	lex.lexStr(file_content);
	ASSERT_TRUE(ctxt.isSafe()) << "Context reported one or more errors while lexing the file. Context log:\n" << ctxt.getLogs();
}

TEST(LexerTests, IncorrectTest1)
{
	std::string file_content, file_path;
	file_path = "lexer/inputs/incorrect_1.fox";
	ASSERT_TRUE(readFileToString(file_path, file_content)) << "Could not open test file \"" << file_path << '"';

	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	ASTContext astctxt;
	Lexer lex(ctxt, astctxt);
	lex.lexStr(file_content);
	EXPECT_FALSE(ctxt.isSafe()) << "Test completed successfully, but was expected to fail.";
}

TEST(LexerTests, IncorrectTest2)
{
	std::string file_content, file_path;
	file_path = "lexer/inputs/incorrect_2.fox";
	ASSERT_TRUE(readFileToString(file_path, file_content)) << "Could not open test file \"" << file_path << '"';

	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	ASTContext astctxt;
	Lexer lex(ctxt, astctxt);
	lex.lexStr(file_content);
	EXPECT_FALSE(ctxt.isSafe()) << "Test completed successfully, but was expected to fail.";
}

TEST(LexerTests, IncorrectTest3)
{
	std::string file_content, file_path;
	file_path = "lexer/inputs/incorrect_3.fox";
	ASSERT_TRUE(readFileToString(file_path, file_content)) << "Could not open test file \"" << file_path << '"';

	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	ASTContext astctxt;
	Lexer lex(ctxt, astctxt);
	lex.lexStr(file_content);
	EXPECT_FALSE(ctxt.isSafe()) << "Test completed successfully, but was expected to fail.";
}

TEST(LexerTests, IncorrectTest4)
{
	std::string file_content, file_path;
	file_path = "lexer/inputs/incorrect_3.fox";
	ASSERT_TRUE(readFileToString(file_path, file_content)) << "Could not open test file \"" << file_path << '"';

	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	ASTContext astctxt;
	Lexer lex(ctxt, astctxt);
	lex.lexStr(file_content);
	EXPECT_FALSE(ctxt.isSafe()) << "Test completed successfully, but was expected to fail.";
}

TEST(TokenTests, FloatID)
{
	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	ASTContext astctxt;

	Token tok1(ctxt, astctxt, "3.14");
	Token tok2(ctxt, astctxt, "0.0");
	Token tok3(ctxt, astctxt, "0.3333333333333");

	ASSERT_TRUE(tok1.isLiteral()) << "Logs:" << ctxt.getLogs();
	ASSERT_TRUE(tok2.isLiteral()) << "Logs:" << ctxt.getLogs();
	ASSERT_TRUE(tok3.isLiteral()) << "Logs:" << ctxt.getLogs();

	LiteralInfo litInfo1 = tok1.getLiteralInfo();
	LiteralInfo litInfo2 = tok2.getLiteralInfo();
	LiteralInfo litInfo3 = tok3.getLiteralInfo();

	ASSERT_TRUE(litInfo1) << "LiteralInfo was null?";
	ASSERT_TRUE(litInfo2) << "LiteralInfo was null?";
	ASSERT_TRUE(litInfo3) << "LiteralInfo was null?";

	ASSERT_TRUE(litInfo1.isFloat() && litInfo1.is<FloatType>());
	ASSERT_TRUE(litInfo2.isFloat() && litInfo2.is<FloatType>());
	ASSERT_TRUE(litInfo3.isFloat() && litInfo3.is<FloatType>());

	EXPECT_EQ(litInfo1.get<FloatType>(), 3.14f) << "Value was not the one expected.";
	EXPECT_EQ(litInfo2.get<FloatType>(), 0.0f) << "Value was not the one expected.";
	EXPECT_EQ(litInfo3.get<FloatType>(), 0.3333333333333f) << "Value was not the one expected.";
}

TEST(TokenTests, IntId)
{
	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	ASTContext astctxt;

	Token tok1(ctxt, astctxt,"0");
	Token tok2(ctxt, astctxt,"9223372036854775000");
	Token tok3(ctxt, astctxt,"4242424242424242");

	ASSERT_TRUE(tok1.isLiteral()) << "Logs:" << ctxt.getLogs();
	ASSERT_TRUE(tok2.isLiteral()) << "Logs:" << ctxt.getLogs();
	ASSERT_TRUE(tok3.isLiteral()) << "Logs:" << ctxt.getLogs();

	LiteralInfo litInfo1 = tok1.getLiteralInfo();
	LiteralInfo litInfo2 = tok2.getLiteralInfo();
	LiteralInfo litInfo3 = tok3.getLiteralInfo();

	ASSERT_TRUE(litInfo1) << "LiteralInfo was null?";
	ASSERT_TRUE(litInfo2) << "LiteralInfo was null?";
	ASSERT_TRUE(litInfo3) << "LiteralInfo was null?";

	ASSERT_TRUE(litInfo1.isInt() && litInfo1.is<IntType>());
	ASSERT_TRUE(litInfo2.isInt() && litInfo2.is<IntType>());
	ASSERT_TRUE(litInfo3.isInt() && litInfo3.is<IntType>());

	EXPECT_EQ(litInfo1.get<IntType>(), 0) << "Value was not the one expected.";
	EXPECT_EQ(litInfo2.get<IntType>(), 9223372036854775000) << "Value was not the one expected.";
	EXPECT_EQ(litInfo3.get<IntType>(), 4242424242424242) << "Value was not the one expected.";
}

TEST(TokenTests, StringID)
{
	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	ASTContext astctxt;

	Token tok1(ctxt, astctxt,"\"Hello, world!\"");
	Token tok2(ctxt, astctxt,"\"\"");
	Token tok3(ctxt, astctxt,"\"!\"");

	ASSERT_TRUE(tok1.isLiteral()) << "Logs:" << ctxt.getLogs();
	ASSERT_TRUE(tok2.isLiteral()) << "Logs:" << ctxt.getLogs();
	ASSERT_TRUE(tok3.isLiteral()) << "Logs:" << ctxt.getLogs();

	LiteralInfo litInfo1 = tok1.getLiteralInfo();
	LiteralInfo litInfo2 = tok2.getLiteralInfo();
	LiteralInfo litInfo3 = tok3.getLiteralInfo();

	ASSERT_TRUE(litInfo1) << "LiteralInfo was null?";
	ASSERT_TRUE(litInfo2) << "LiteralInfo was null?";
	ASSERT_TRUE(litInfo3) << "LiteralInfo was null?";

	ASSERT_TRUE(litInfo1.isString() && litInfo1.is<std::string>());
	ASSERT_TRUE(litInfo2.isString() && litInfo2.is<std::string>());
	ASSERT_TRUE(litInfo3.isString() && litInfo3.is<std::string>());

	EXPECT_EQ(litInfo1.get<std::string>(), "Hello, world!") << "Value was not the one expected.";
	EXPECT_EQ(litInfo2.get<std::string>(), "") << "Value was not the one expected.";
	EXPECT_EQ(litInfo3.get<std::string>(), "!") << "Value was not the one expected.";
}

TEST(TokenTests, CharID)
{
	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	ASTContext astctxt;

	Token tok1(ctxt, astctxt,"'c'");
	Token tok2(ctxt, astctxt,"' '");
	Token tok3(ctxt, astctxt,"'!'");

	ASSERT_TRUE(tok1.isLiteral()) << "Logs:" << ctxt.getLogs();
	ASSERT_TRUE(tok2.isLiteral()) << "Logs:" << ctxt.getLogs();
	ASSERT_TRUE(tok3.isLiteral()) << "Logs:" << ctxt.getLogs();

	LiteralInfo litInfo1 = tok1.getLiteralInfo();
	LiteralInfo litInfo2 = tok2.getLiteralInfo();
	LiteralInfo litInfo3 = tok3.getLiteralInfo();

	ASSERT_TRUE(litInfo1) << "LiteralInfo was null?";
	ASSERT_TRUE(litInfo2) << "LiteralInfo was null?";
	ASSERT_TRUE(litInfo3) << "LiteralInfo was null?";

	ASSERT_TRUE(litInfo1.isChar() && litInfo1.is<CharType>());
	ASSERT_TRUE(litInfo2.isChar() && litInfo2.is<CharType>());
	ASSERT_TRUE(litInfo3.isChar() && litInfo3.is<CharType>());

	EXPECT_EQ(litInfo1.get<CharType>(), 'c') << "Value was not the one expected.";
	EXPECT_EQ(litInfo2.get<CharType>(), ' ') << "Value was not the one expected.";
	EXPECT_EQ(litInfo3.get<CharType>(), '!') << "Value was not the one expected.";
}

TEST(TokenTests, BoolID)
{
	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	ASTContext astctxt;

	Token tok1(ctxt, astctxt,"true");
	Token tok2(ctxt, astctxt,"false");

	ASSERT_TRUE(tok1.isLiteral()) << "Logs:" << ctxt.getLogs();
	ASSERT_TRUE(tok2.isLiteral()) << "Logs:" << ctxt.getLogs();

	LiteralInfo litInfo1 = tok1.getLiteralInfo();
	LiteralInfo litInfo2 = tok2.getLiteralInfo();

	ASSERT_TRUE(litInfo1) << "LiteralInfo was null?";
	ASSERT_TRUE(litInfo2) << "LiteralInfo was null?";

	ASSERT_TRUE(litInfo1.isBool() && litInfo1.is<bool>());
	ASSERT_TRUE(litInfo2.isBool() && litInfo2.is<bool>());

	EXPECT_TRUE(litInfo1.get<bool>()) << "Value was not the one expected.";
	EXPECT_FALSE(litInfo2.get<bool>()) << "Value was not the one expected.";
}