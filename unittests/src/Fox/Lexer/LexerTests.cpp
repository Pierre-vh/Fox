//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : LexerTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  (Unit) Tests for the Lexer.
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/AST/ASTContext.hpp" 
#include "Fox/Lexer/Lexer.hpp"
#include "Fox/Lexer/Token.hpp"
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
  auto file = srcMgr.loadFromFile(getPath("lexer/inputs/correct_1.fox"));
  ASSERT_TRUE(file) << "Could not open test file";
  lexer.lexFile(file);
  EXPECT_FALSE(ctxt.hadErrors());
}

TEST_F(LexerTest, IncorrectTest1) {
  auto file = srcMgr.loadFromFile(getPath("lexer/inputs/incorrect_1.fox"));
  ASSERT_TRUE(file) << "Could not open test file";
  lexer.lexFile(file);
  EXPECT_TRUE(ctxt.hadErrors());
}

TEST_F(LexerTest, IncorrectTest2) {
  auto file = srcMgr.loadFromFile(getPath("lexer/inputs/incorrect_2.fox"));
  ASSERT_TRUE(file) << "Could not open test file";
  lexer.lexFile(file);
  EXPECT_TRUE(ctxt.hadErrors());
}

TEST_F(LexerTest, IncorrectTest3) {
  auto file = srcMgr.loadFromFile(getPath("lexer/inputs/incorrect_3.fox"));
  ASSERT_TRUE(file) << "Could not open test file";
  lexer.lexFile(file);
  EXPECT_TRUE(ctxt.hadErrors());
}

TEST_F(LexerTest, IncorrectTest4) {
  auto file = srcMgr.loadFromFile(getPath("lexer/inputs/incorrect_4.fox"));
  ASSERT_TRUE(file) << "Could not open test file";
  lexer.lexFile(file);
  EXPECT_TRUE(ctxt.hadErrors());
}

TEST_F(LexerTest, FloatTokens) {

  Token tok1(ctxt, "3.14");
  Token tok2(ctxt, "0.0");
  Token tok3(ctxt, "0.3333333333333");

  ASSERT_TRUE(tok1.isLiteral());
  ASSERT_TRUE(tok2.isLiteral());
  ASSERT_TRUE(tok3.isLiteral());

  LiteralInfo litInfo1 = tok1.getLiteralInfo();
  LiteralInfo litInfo2 = tok2.getLiteralInfo();
  LiteralInfo litInfo3 = tok3.getLiteralInfo();

  ASSERT_FALSE(litInfo1.isNull());
  ASSERT_FALSE(litInfo2.isNull());
  ASSERT_FALSE(litInfo3.isNull());

  EXPECT_TRUE(litInfo1.isFloat() && litInfo1.is<FoxDouble>());
  EXPECT_TRUE(litInfo2.isFloat() && litInfo2.is<FoxDouble>());
  EXPECT_TRUE(litInfo3.isFloat() && litInfo3.is<FoxDouble>());

  EXPECT_EQ(litInfo1.get<FoxDouble>(), 3.14f);
  EXPECT_EQ(litInfo2.get<FoxDouble>(), 0.0f);
  EXPECT_EQ(litInfo3.get<FoxDouble>(), 0.3333333333333f);
}

TEST_F(LexerTest, IntTokens) {
  Token tok1(ctxt,"0");
  Token tok2(ctxt,"9223372036854775000");
  Token tok3(ctxt,"4242424242424242");

  ASSERT_TRUE(tok1.isLiteral());
  ASSERT_TRUE(tok2.isLiteral());
  ASSERT_TRUE(tok3.isLiteral());

  LiteralInfo litInfo1 = tok1.getLiteralInfo();
  LiteralInfo litInfo2 = tok2.getLiteralInfo();
  LiteralInfo litInfo3 = tok3.getLiteralInfo();

  ASSERT_FALSE(litInfo1.isNull());
  ASSERT_FALSE(litInfo2.isNull());
  ASSERT_FALSE(litInfo3.isNull());

  ASSERT_TRUE(litInfo1.isInt() && litInfo1.is<FoxInt>());
  ASSERT_TRUE(litInfo2.isInt() && litInfo2.is<FoxInt>());
  ASSERT_TRUE(litInfo3.isInt() && litInfo3.is<FoxInt>());

  EXPECT_EQ(litInfo1.get<FoxInt>(), 0);
  EXPECT_EQ(litInfo2.get<FoxInt>(), 9223372036854775000);
  EXPECT_EQ(litInfo3.get<FoxInt>(), 4242424242424242);
}

TEST_F(LexerTest, StringTokens) {
  Token tok1(ctxt, "\"Hello, world!\"");
  Token tok2(ctxt, "\"\"");
  Token tok3(ctxt, "\"!\"");

  ASSERT_TRUE(tok1.isLiteral());
  ASSERT_TRUE(tok2.isLiteral());
  ASSERT_TRUE(tok3.isLiteral());

  LiteralInfo litInfo1 = tok1.getLiteralInfo();
  LiteralInfo litInfo2 = tok2.getLiteralInfo();
  LiteralInfo litInfo3 = tok3.getLiteralInfo();

  ASSERT_FALSE(litInfo1.isNull());
  ASSERT_FALSE(litInfo2.isNull());
  ASSERT_FALSE(litInfo3.isNull());

  ASSERT_TRUE(litInfo1.isString() && litInfo1.is<std::string>());
  ASSERT_TRUE(litInfo2.isString() && litInfo2.is<std::string>());
  ASSERT_TRUE(litInfo3.isString() && litInfo3.is<std::string>());

  EXPECT_EQ(litInfo1.get<std::string>(), "Hello, world!");
  EXPECT_EQ(litInfo2.get<std::string>(), "");
  EXPECT_EQ(litInfo3.get<std::string>(), "!");
}

TEST_F(LexerTest, CharTokens) {
  Token tok1(ctxt, "'c'");
  Token tok2(ctxt, "' '");
  Token tok3(ctxt, "'!'");

  ASSERT_TRUE(tok1.isLiteral());
  ASSERT_TRUE(tok2.isLiteral());
  ASSERT_TRUE(tok3.isLiteral());

  LiteralInfo litInfo1 = tok1.getLiteralInfo();
  LiteralInfo litInfo2 = tok2.getLiteralInfo();
  LiteralInfo litInfo3 = tok3.getLiteralInfo();

  ASSERT_FALSE(litInfo1.isNull());
  ASSERT_FALSE(litInfo2.isNull());
  ASSERT_FALSE(litInfo3.isNull());

  ASSERT_TRUE(litInfo1.isChar() && litInfo1.is<FoxChar>());
  ASSERT_TRUE(litInfo2.isChar() && litInfo2.is<FoxChar>());
  ASSERT_TRUE(litInfo3.isChar() && litInfo3.is<FoxChar>());

  EXPECT_EQ(litInfo1.get<FoxChar>(), 'c');
  EXPECT_EQ(litInfo2.get<FoxChar>(), ' ');
  EXPECT_EQ(litInfo3.get<FoxChar>(), '!');
}

TEST_F(LexerTest, BoolTokens) {
  Token tok1(ctxt, "true");
  Token tok2(ctxt, "false");

  ASSERT_TRUE(tok1.isLiteral());
  ASSERT_TRUE(tok2.isLiteral());

  LiteralInfo litInfo1 = tok1.getLiteralInfo();
  LiteralInfo litInfo2 = tok2.getLiteralInfo();

  ASSERT_FALSE(litInfo1.isNull());
  ASSERT_FALSE(litInfo2.isNull());

  ASSERT_TRUE(litInfo1.isBool() && litInfo1.is<bool>());
  ASSERT_TRUE(litInfo2.isBool() && litInfo2.is<bool>());

  EXPECT_TRUE(litInfo1.get<bool>());
  EXPECT_FALSE(litInfo2.get<bool>());
}

TEST_F(LexerTest, Coordinates1) {
  std::string file_content, file_path;
  auto file = srcMgr.loadFromFile(getPath("lexer/coordtests/test1.fox"));
  ASSERT_TRUE(file) << "Could not open test file";

  lexer.lexFile(file);
  ASSERT_FALSE(ctxt.hadErrors());

  TokenVector& output = lexer.getTokenVector();
  char varFounds = 0;
  for (const Token& elem : output) {
    if (elem.getAsString() == "_FIRST_VARIABLE_") {
      varFounds++;
      auto beg_ploc = srcMgr.getCompleteLoc(elem.getRange().getBegin());
      auto end_ploc = srcMgr.getCompleteLoc(elem.getRange().getEnd());
      
      // Line
      EXPECT_EQ(beg_ploc.line, 7);
      EXPECT_EQ(end_ploc.line, 7);

      // Col
      EXPECT_EQ(beg_ploc.column, 5);
      EXPECT_EQ(end_ploc.column, 20);
    }
    else if (elem.getAsString() == "_2NDVAR__") {
      varFounds++;
      auto beg_ploc = srcMgr.getCompleteLoc(elem.getRange().getBegin());
      auto end_ploc = srcMgr.getCompleteLoc(elem.getRange().getEnd());

      // Line
      EXPECT_EQ(beg_ploc.line, 10);
      EXPECT_EQ(end_ploc.line, 10);

      // Col
      EXPECT_EQ(beg_ploc.column, 7);
      EXPECT_EQ(end_ploc.column, 15);
    }
    else if (elem.getAsString() == "ThirdVariable") {
      varFounds++;
      auto beg_ploc = srcMgr.getCompleteLoc(elem.getRange().getBegin());
      auto end_ploc = srcMgr.getCompleteLoc(elem.getRange().getEnd());

      // Line
      EXPECT_EQ(beg_ploc.line, 13);
      EXPECT_EQ(end_ploc.line, 13);

      // Col
      EXPECT_EQ(beg_ploc.column, 5);
      EXPECT_EQ(end_ploc.column, 17);
    }
  }
  EXPECT_EQ(varFounds, 3) << "Did not find all 3 variables";
}
