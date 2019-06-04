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
#include <iostream>

using namespace fox;
using namespace fox::test;

namespace {
  class LexerTest : public testing::Test {
    public:
      LexerTest() : diagEngine(sourceMgr, std::cout) {}

      Lexer createLexer(FileID file) {
        return Lexer(sourceMgr, diagEngine, file);
      }

    protected:
      SourceManager sourceMgr;
      DiagnosticEngine diagEngine;
  };
}

TEST_F(LexerTest,CorrectTest1) {
  auto fullPath = getPath("lexer/inputs/correct_1.fox");
  auto result = sourceMgr.readFile(fullPath);
  FileID file = result.first;
  ASSERT_TRUE(file) << "Could not open test file '" << fullPath << 
    "'\n\tReason:" << to_string(result.second);
  createLexer(file).lex();
  EXPECT_FALSE(diagEngine.hadAnyError());
}

TEST_F(LexerTest, IncorrectTest1) {
  auto fullPath = getPath("lexer/inputs/incorrect_1.fox");
  auto result = sourceMgr.readFile(fullPath);
  FileID file = result.first;
  ASSERT_TRUE(file) << "Could not open test file '" << fullPath << 
    "'\n\tReason:" << to_string(result.second);
  createLexer(file).lex();
  EXPECT_TRUE(diagEngine.hadAnyError());
}

TEST_F(LexerTest, IncorrectTest2) {
  auto fullPath = getPath("lexer/inputs/incorrect_2.fox");
  auto result = sourceMgr.readFile(fullPath);
  FileID file = result.first;
  ASSERT_TRUE(file) << "Could not open test file '" << fullPath << 
    "'\n\tReason:" << to_string(result.second);
  createLexer(file).lex();
  EXPECT_TRUE(diagEngine.hadAnyError());
}

TEST_F(LexerTest, IncorrectTest3) {
  auto fullPath = getPath("lexer/inputs/incorrect_3.fox");
  auto result = sourceMgr.readFile(fullPath);
  FileID file = result.first;
  ASSERT_TRUE(file) << "Could not open test file '" << fullPath << 
    "'\n\tReason:" << to_string(result.second);
  createLexer(file).lex();
  EXPECT_TRUE(diagEngine.hadAnyError());
}

TEST_F(LexerTest, IncorrectTest4) {
  auto fullPath = getPath("lexer/inputs/incorrect_4.fox");
  auto result = sourceMgr.readFile(fullPath);
  FileID file = result.first;
  ASSERT_TRUE(file) << "Could not open test file '" << fullPath << 
    "'\n\tReason:" << to_string(result.second);
  createLexer(file).lex();
  EXPECT_TRUE(diagEngine.hadAnyError());
}