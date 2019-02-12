//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : LexerTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  (Unit) Tests for the Lexer.
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/AST/ASTContext.hpp" 
#include "Fox/Lexer/Lexer.hpp"
#include "Fox/Lexer/Token.hpp"
#include "Fox/Common/SourceManager.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Support/TestUtils.hpp"

using namespace fox;
using namespace fox::test;

namespace {
  class LexerTest : public testing::Test {
    public:
      LexerTest() : diags(srcMgr), ctxt(srcMgr, diags), lexer(ctxt) {}

    protected:
      // C++ Standard 12.6.2.10:
      //    [...] non-static data members are initialized in the order
      //    they were declared in the class definition [...]
      //
      // So here, the order of declaration of the members must be strictly
      // respected, because:
      //    diags depends on srcMgr
      //    ctxt depends on both srcMgr and diags
      //    lexer depends on ctxt.
      SourceManager srcMgr;
      DiagnosticEngine diags;
      ASTContext ctxt;
      Lexer lexer;
  };
}

TEST_F(LexerTest,CorrectTest1) {
  auto fullPath = getPath("lexer/inputs/correct_1.fox");
  auto result = srcMgr.readFile(fullPath);
  FileID file = result.first;
  ASSERT_TRUE(file) << "Could not open test file '" << fullPath << 
    "'\n\tReason:" << toString(result.second);
  lexer.lexFile(file);
  EXPECT_FALSE(ctxt.diagEngine.hadAnyError());
}

TEST_F(LexerTest, IncorrectTest1) {
  auto fullPath = getPath("lexer/inputs/incorrect_1.fox");
  auto result = srcMgr.readFile(fullPath);
  FileID file = result.first;
  ASSERT_TRUE(file) << "Could not open test file '" << fullPath << 
    "'\n\tReason:" << toString(result.second);  lexer.lexFile(file);
  EXPECT_TRUE(ctxt.diagEngine.hadAnyError());
}

TEST_F(LexerTest, IncorrectTest2) {
  auto fullPath = getPath("lexer/inputs/incorrect_2.fox");
  auto result = srcMgr.readFile(fullPath);
  FileID file = result.first;
  ASSERT_TRUE(file) << "Could not open test file '" << fullPath << 
    "'\n\tReason:" << toString(result.second);  lexer.lexFile(file);
  EXPECT_TRUE(ctxt.diagEngine.hadAnyError());
}

TEST_F(LexerTest, IncorrectTest3) {
  auto fullPath = getPath("lexer/inputs/incorrect_3.fox");
  auto result = srcMgr.readFile(fullPath);
  FileID file = result.first;
  ASSERT_TRUE(file) << "Could not open test file '" << fullPath << 
    "'\n\tReason:" << toString(result.second);  lexer.lexFile(file);
  EXPECT_TRUE(ctxt.diagEngine.hadAnyError());
}

TEST_F(LexerTest, IncorrectTest4) {
  auto fullPath = getPath("lexer/inputs/incorrect_4.fox");
  auto result = srcMgr.readFile(fullPath);
  FileID file = result.first;
  ASSERT_TRUE(file) << "Could not open test file '" << fullPath << 
    "'\n\tReason:" << toString(result.second);  lexer.lexFile(file);
  EXPECT_TRUE(ctxt.diagEngine.hadAnyError());
}

TEST_F(LexerTest, DoubleTokens) {
  Token tok1(ctxt, "3.14");
  Token tok2(ctxt, "0.0");
  Token tok3(ctxt, "0.3333333333333");

  ASSERT_TRUE(tok1.isLiteral() && tok1.isDoubleLiteral());
  ASSERT_TRUE(tok2.isLiteral() && tok2.isDoubleLiteral());
  ASSERT_TRUE(tok3.isLiteral() && tok3.isDoubleLiteral());

  EXPECT_EQ(tok1.getDoubleValue(), 3.14);
  EXPECT_EQ(tok2.getDoubleValue(), 0.0);
  EXPECT_EQ(tok3.getDoubleValue(), 0.3333333333333);
}

TEST_F(LexerTest, IntTokens) {
  Token tok1(ctxt,"0");
  Token tok2(ctxt,"9223372036854775000");
  Token tok3(ctxt,"4242424242424242");

  ASSERT_TRUE(tok1.isLiteral() && tok1.isIntLiteral());
  ASSERT_TRUE(tok2.isLiteral() && tok2.isIntLiteral());
  ASSERT_TRUE(tok3.isLiteral() && tok3.isIntLiteral());

  EXPECT_EQ(tok1.getIntValue(), 0);
  EXPECT_EQ(tok2.getIntValue(), 9223372036854775000);
  EXPECT_EQ(tok3.getIntValue(), 4242424242424242);
}

TEST_F(LexerTest, StringTokens) {
  Token tok1(ctxt, "\"Hello, world!\"");
  Token tok2(ctxt, "\"\"");
  Token tok3(ctxt, "\"!\"");

  ASSERT_TRUE(tok1.isLiteral() && tok1.isStringLiteral());
  ASSERT_TRUE(tok2.isLiteral() && tok2.isStringLiteral());
  ASSERT_TRUE(tok3.isLiteral() && tok3.isStringLiteral());

  EXPECT_EQ(tok1.getStringValue(), "Hello, world!");
  EXPECT_EQ(tok2.getStringValue(), "");
  EXPECT_EQ(tok3.getStringValue(), "!");
}

TEST_F(LexerTest, CharTokens) {
  Token tok1(ctxt, "'c'");
  Token tok2(ctxt, "' '");
  Token tok3(ctxt, "'!'");

  ASSERT_TRUE(tok1.isLiteral() && tok1.isCharLiteral());
  ASSERT_TRUE(tok2.isLiteral() && tok2.isCharLiteral());
  ASSERT_TRUE(tok3.isLiteral() && tok3.isCharLiteral());

  EXPECT_EQ(tok1.getCharValue(), (FoxChar)'c');
  EXPECT_EQ(tok2.getCharValue(), (FoxChar)' ');
  EXPECT_EQ(tok3.getCharValue(), (FoxChar)'!');
}

TEST_F(LexerTest, BoolTokens) {
  Token tok1(ctxt, "true");
  Token tok2(ctxt, "false");

  ASSERT_TRUE(tok1.isLiteral() && tok1.isBoolLiteral());
  ASSERT_TRUE(tok2.isLiteral() && tok2.isBoolLiteral());

  EXPECT_TRUE(tok1.getBoolValue());
  EXPECT_FALSE(tok2.getBoolValue());
}

TEST_F(LexerTest, Coordinates1) {
  auto fullPath = getPath("lexer/coordtests/test1.fox");
  auto result = srcMgr.readFile(fullPath);
  FileID file = result.first;
  ASSERT_TRUE(file) << "Could not open test file '" << fullPath << "'"
    << "\n\tReason:" << toString(result.second);
  lexer.lexFile(file);
  ASSERT_FALSE(ctxt.diagEngine.hadAnyError());

  TokenVector& output = lexer.getTokenVector();
  char varFounds = 0;
  for (const Token& elem : output) {
    printf(elem.getAsString().c_str());
    if (elem.getAsString() == "_FIRST_VARIABLE_") {
      varFounds++;
      auto beg_ploc = srcMgr.getCompleteLoc(elem.getRange().getBegin());
      auto end_ploc = srcMgr.getCompleteLoc(elem.getRange().getEnd());
      
      // Line
      EXPECT_EQ(beg_ploc.line, 7u);
      EXPECT_EQ(end_ploc.line, 7u);

      // Col
      EXPECT_EQ(beg_ploc.column, 5u);
      EXPECT_EQ(end_ploc.column, 20u);
    }
    else if (elem.getAsString() == "_2NDVAR__") {
      varFounds++;
      auto beg_ploc = srcMgr.getCompleteLoc(elem.getRange().getBegin());
      auto end_ploc = srcMgr.getCompleteLoc(elem.getRange().getEnd());

      // Line
      EXPECT_EQ(beg_ploc.line, 10u);
      EXPECT_EQ(end_ploc.line, 10u);

      // Col
      EXPECT_EQ(beg_ploc.column, 7u);
      EXPECT_EQ(end_ploc.column, 15u);
    }
    else if (elem.getAsString() == "ThirdVariable") {
      varFounds++;
      auto beg_ploc = srcMgr.getCompleteLoc(elem.getRange().getBegin());
      auto end_ploc = srcMgr.getCompleteLoc(elem.getRange().getEnd());

      // Line
      EXPECT_EQ(beg_ploc.line, 13u);
      EXPECT_EQ(end_ploc.line, 13u);

      // Col
      EXPECT_EQ(beg_ploc.column, 5u);
      EXPECT_EQ(end_ploc.column, 17u);
    }
  }
  EXPECT_EQ(varFounds, 3) << "Did not find all 3 variables";
}
