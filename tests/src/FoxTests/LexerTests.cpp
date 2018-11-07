//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : LexerTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  (Unit) Tests for the Lexer.
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Support/TestUtils.hpp"
#include "Fox/AST/ASTContext.hpp" 
#include "Fox/Lexer/Lexer.hpp"
#include "Fox/Lexer/Token.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include <memory>
using namespace fox;
using namespace fox::test;

TEST(LexerTests,CorrectTest1) {
  SourceManager sm;
  DiagnosticEngine dg(sm);

  auto file = sm.loadFromFile(convertRelativeTestResPathToAbsolute("lexer/inputs/correct_1.fox"));
  ASSERT_TRUE(file) << "Could not open test file";
  ASTContext astctxt;
  Lexer lex(dg,sm, astctxt);
  lex.lexFile(file);

  EXPECT_TRUE(dg.getErrorsCount() == 0) << "One or more errors occured while lexing the file";
}

TEST(LexerTests, IncorrectTest1) {
  SourceManager sm;
  DiagnosticEngine dg(sm);

  auto file = sm.loadFromFile(convertRelativeTestResPathToAbsolute("lexer/inputs/incorrect_1.fox"));
  ASSERT_TRUE(file) << "Could not open test file";
  ASTContext astctxt;
  Lexer lex(dg, sm, astctxt);
  lex.lexFile(file);

  EXPECT_TRUE(dg.getErrorsCount() != 0) << "No error occured while lexing the file, but the test expected errors to occur.";
}

TEST(LexerTests, IncorrectTest2) {
  SourceManager sm;
  DiagnosticEngine dg(sm);

  auto file = sm.loadFromFile(convertRelativeTestResPathToAbsolute("lexer/inputs/incorrect_2.fox"));
  ASSERT_TRUE(file) << "Could not open test file";
  ASTContext astctxt;
  Lexer lex(dg, sm, astctxt);
  lex.lexFile(file);

  EXPECT_TRUE(dg.getErrorsCount() != 0) << "No error occured while lexing the file, but the test expected errors to occur.";
}

TEST(LexerTests, IncorrectTest3) {
  SourceManager sm;
  DiagnosticEngine dg(sm);

  auto file = sm.loadFromFile(convertRelativeTestResPathToAbsolute("lexer/inputs/incorrect_3.fox"));
  ASSERT_TRUE(file) << "Could not open test file";
  ASTContext astctxt;
  Lexer lex(dg, sm, astctxt);
  lex.lexFile(file);

  EXPECT_TRUE(dg.getErrorsCount() != 0) << "No error occured while lexing the file, but the test expected errors to occur.";
}

TEST(LexerTests, IncorrectTest4) {
  SourceManager sm;
  DiagnosticEngine dg(sm);

  auto file = sm.loadFromFile(convertRelativeTestResPathToAbsolute("lexer/inputs/incorrect_4.fox"));
  ASSERT_TRUE(file) << "Could not open test file";
  ASTContext astctxt;
  Lexer lex(dg, sm, astctxt);
  lex.lexFile(file);

  EXPECT_TRUE(dg.getErrorsCount() != 0) << "No error occured while lexing the file, but the test expected errors to occur.";
}

TEST(TokenTests, FloatID) {
  SourceManager sm;
  DiagnosticEngine dg(sm);
  ASTContext astctxt;

  Token tok1(dg, astctxt, "3.14");
  Token tok2(dg, astctxt, "0.0");
  Token tok3(dg, astctxt, "0.3333333333333");

  ASSERT_TRUE(tok1.isLiteral());
  ASSERT_TRUE(tok2.isLiteral());
  ASSERT_TRUE(tok3.isLiteral());

  LiteralInfo litInfo1 = tok1.getLiteralInfo();
  LiteralInfo litInfo2 = tok2.getLiteralInfo();
  LiteralInfo litInfo3 = tok3.getLiteralInfo();

  ASSERT_FALSE(litInfo1.isNull()) << "LiteralInfo was null?";
  ASSERT_FALSE(litInfo2.isNull()) << "LiteralInfo was null?";
  ASSERT_FALSE(litInfo3.isNull()) << "LiteralInfo was null?";

  EXPECT_TRUE(litInfo1.isFloat() && litInfo1.is<FoxFloat>());
  EXPECT_TRUE(litInfo2.isFloat() && litInfo2.is<FoxFloat>());
  EXPECT_TRUE(litInfo3.isFloat() && litInfo3.is<FoxFloat>());

  EXPECT_EQ(litInfo1.get<FoxFloat>(), 3.14f) << "Value was not the one expected.";
  EXPECT_EQ(litInfo2.get<FoxFloat>(), 0.0f) << "Value was not the one expected.";
  EXPECT_EQ(litInfo3.get<FoxFloat>(), 0.3333333333333f) << "Value was not the one expected.";
}

TEST(TokenTests, IntId) {
  SourceManager sm;
  DiagnosticEngine dg(sm);
  ASTContext astctxt;

  Token tok1(dg, astctxt,"0");
  Token tok2(dg, astctxt,"9223372036854775000");
  Token tok3(dg, astctxt,"4242424242424242");

  ASSERT_TRUE(tok1.isLiteral());
  ASSERT_TRUE(tok2.isLiteral());
  ASSERT_TRUE(tok3.isLiteral());

  LiteralInfo litInfo1 = tok1.getLiteralInfo();
  LiteralInfo litInfo2 = tok2.getLiteralInfo();
  LiteralInfo litInfo3 = tok3.getLiteralInfo();

  ASSERT_FALSE(litInfo1.isNull()) << "LiteralInfo was null?";
  ASSERT_FALSE(litInfo2.isNull()) << "LiteralInfo was null?";
  ASSERT_FALSE(litInfo3.isNull()) << "LiteralInfo was null?";

  ASSERT_TRUE(litInfo1.isInt() && litInfo1.is<FoxInt>());
  ASSERT_TRUE(litInfo2.isInt() && litInfo2.is<FoxInt>());
  ASSERT_TRUE(litInfo3.isInt() && litInfo3.is<FoxInt>());

  EXPECT_EQ(litInfo1.get<FoxInt>(), 0) << "Value was not the one expected.";
  EXPECT_EQ(litInfo2.get<FoxInt>(), 9223372036854775000) << "Value was not the one expected.";
  EXPECT_EQ(litInfo3.get<FoxInt>(), 4242424242424242) << "Value was not the one expected.";
}

TEST(TokenTests, StringID) {
  SourceManager sm;
  DiagnosticEngine dg(sm);
  ASTContext astctxt;

  Token tok1(dg, astctxt,"\"Hello, world!\"");
  Token tok2(dg, astctxt,"\"\"");
  Token tok3(dg, astctxt,"\"!\"");

  ASSERT_TRUE(tok1.isLiteral());
  ASSERT_TRUE(tok2.isLiteral());
  ASSERT_TRUE(tok3.isLiteral());

  LiteralInfo litInfo1 = tok1.getLiteralInfo();
  LiteralInfo litInfo2 = tok2.getLiteralInfo();
  LiteralInfo litInfo3 = tok3.getLiteralInfo();

  ASSERT_FALSE(litInfo1.isNull()) << "LiteralInfo was null?";
  ASSERT_FALSE(litInfo2.isNull()) << "LiteralInfo was null?";
  ASSERT_FALSE(litInfo3.isNull()) << "LiteralInfo was null?";

  ASSERT_TRUE(litInfo1.isString() && litInfo1.is<std::string>());
  ASSERT_TRUE(litInfo2.isString() && litInfo2.is<std::string>());
  ASSERT_TRUE(litInfo3.isString() && litInfo3.is<std::string>());

  EXPECT_EQ(litInfo1.get<std::string>(), "Hello, world!") << "Value was not the one expected.";
  EXPECT_EQ(litInfo2.get<std::string>(), "") << "Value was not the one expected.";
  EXPECT_EQ(litInfo3.get<std::string>(), "!") << "Value was not the one expected.";
}

TEST(TokenTests, CharID) {
  SourceManager sm;
  DiagnosticEngine dg(sm);
  ASTContext astctxt;

  Token tok1(dg, astctxt,"'c'");
  Token tok2(dg, astctxt,"' '");
  Token tok3(dg, astctxt,"'!'");

  ASSERT_TRUE(tok1.isLiteral());
  ASSERT_TRUE(tok2.isLiteral());
  ASSERT_TRUE(tok3.isLiteral());

  LiteralInfo litInfo1 = tok1.getLiteralInfo();
  LiteralInfo litInfo2 = tok2.getLiteralInfo();
  LiteralInfo litInfo3 = tok3.getLiteralInfo();

  ASSERT_FALSE(litInfo1.isNull()) << "LiteralInfo was null?";
  ASSERT_FALSE(litInfo2.isNull()) << "LiteralInfo was null?";
  ASSERT_FALSE(litInfo3.isNull()) << "LiteralInfo was null?";

  ASSERT_TRUE(litInfo1.isChar() && litInfo1.is<FoxChar>());
  ASSERT_TRUE(litInfo2.isChar() && litInfo2.is<FoxChar>());
  ASSERT_TRUE(litInfo3.isChar() && litInfo3.is<FoxChar>());

  EXPECT_EQ(litInfo1.get<FoxChar>(), 'c') << "Value was not the one expected.";
  EXPECT_EQ(litInfo2.get<FoxChar>(), ' ') << "Value was not the one expected.";
  EXPECT_EQ(litInfo3.get<FoxChar>(), '!') << "Value was not the one expected.";
}

TEST(TokenTests, BoolID) {
  SourceManager sm;
  DiagnosticEngine dg(sm);
  ASTContext astctxt;

  Token tok1(dg, astctxt,"true");
  Token tok2(dg, astctxt,"false");

  ASSERT_TRUE(tok1.isLiteral());
  ASSERT_TRUE(tok2.isLiteral());

  LiteralInfo litInfo1 = tok1.getLiteralInfo();
  LiteralInfo litInfo2 = tok2.getLiteralInfo();

  ASSERT_FALSE(litInfo1.isNull()) << "LiteralInfo was null?";
  ASSERT_FALSE(litInfo2.isNull()) << "LiteralInfo was null?";

  ASSERT_TRUE(litInfo1.isBool() && litInfo1.is<bool>());
  ASSERT_TRUE(litInfo2.isBool() && litInfo2.is<bool>());

  EXPECT_TRUE(litInfo1.get<bool>()) << "Value was not the one expected.";
  EXPECT_FALSE(litInfo2.get<bool>()) << "Value was not the one expected.";
}

TEST(LexerTests, Coordinates1) {
  SourceManager sm;
  DiagnosticEngine dg(sm);

  std::string file_content, file_path;
  auto file = sm.loadFromFile(convertRelativeTestResPathToAbsolute("lexer/coordtests/test1.fox"));
  ASSERT_TRUE(file) << "Could not open test file";

  ASTContext astctxt;
  Lexer lex(dg,sm,astctxt);
  lex.lexFile(file);
  ASSERT_TRUE(dg.getErrorsCount() == 0);

  TokenVector& output = lex.getTokenVector();
  char varFounds = 0;
  for (const Token& elem : output) {
    if (elem.getAsString() == "_FIRST_VARIABLE_") {
      varFounds++;
      auto beg_ploc = sm.getCompleteLoc(elem.getRange().getBegin());
      auto end_ploc = sm.getCompleteLoc(elem.getRange().getEnd());
      
      // Line
      EXPECT_EQ(beg_ploc.line, 7);
      EXPECT_EQ(end_ploc.line, 7);

      // Col
      EXPECT_EQ(beg_ploc.column, 5);
      EXPECT_EQ(end_ploc.column, 20);
    }
    else if (elem.getAsString() == "_2NDVAR__") {
      varFounds++;
      auto beg_ploc = sm.getCompleteLoc(elem.getRange().getBegin());
      auto end_ploc = sm.getCompleteLoc(elem.getRange().getEnd());

      // Line
      EXPECT_EQ(beg_ploc.line, 10);
      EXPECT_EQ(end_ploc.line, 10);

      // Col
      EXPECT_EQ(beg_ploc.column, 6);
      EXPECT_EQ(end_ploc.column, 14);
    }
    else if (elem.getAsString() == "ThirdVariable") {
      varFounds++;
      auto beg_ploc = sm.getCompleteLoc(elem.getRange().getBegin());
      auto end_ploc = sm.getCompleteLoc(elem.getRange().getEnd());

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
