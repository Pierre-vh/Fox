//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : UTF8Tests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// (Unit) Tests for the UTF8 String manipulator.
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Support/TestUtils.hpp"
#include "Fox/Common/StringManipulator.hpp"
#include "llvm/ADT/Optional.h"
#include <cwctype>    // std::iswspace

using namespace fox;
using namespace fox::test;

struct TextStats {
  unsigned linecount = 1;
  unsigned charcount = 0;
  unsigned spacecount = 0;
  bool seenCRLF = false;
};

llvm::Optional<TextStats> 
getTextStats(StringManipulator &manip) {
  TextStats rtr;
  try {
    while (!manip.eof()) {
      const auto cur = manip.getCurrentChar();
      if (cur == '\n')
        ++rtr.linecount;
      else if (cur == '\r' && manip.peekNext() == '\n') {
        rtr.seenCRLF = true;
        manip.advance();
        ++rtr.linecount;
      }
      else {
        if (std::iswspace(static_cast<wchar_t>(cur)))
          ++rtr.spacecount;
        ++rtr.charcount;
      }
      manip.advance();
    }
  }
  catch (std::exception&) {
    return llvm::None;
  }
  return rtr;
}

TEST(UTF8Tests,BronzeHorseman) {
  // Open test file
  std::string file_content;
  std::string file_path("lexer/utf8/bronzehorseman.txt");
  ASSERT_TRUE(readFileToString(file_path, file_content)) 
    << "Could not open test file \"" << file_path << '"';

  // Prepare string manipulator & other variables
  StringManipulator manip(file_content);

  // Get text statistics
  auto statsResults = getTextStats(manip);
  ASSERT_TRUE(statsResults.hasValue()) 
    << "An error occured while calculating text statistics";
  auto stats = statsResults.getValue();

  // Expected text statistics
  // 11 Lines
  // 278 Characters
  // 44 Spaces
  // 511 Bytes (501 if LF line endings)
  // 288 Codepoints (278 if LF line endings)
  EXPECT_EQ(11u, stats.linecount);
  EXPECT_EQ(268u, stats.charcount);
  EXPECT_EQ(34u, stats.spacecount);
  // Remove 10 from the values if we don't use CRLF line endings.
  EXPECT_EQ(511u - (stats.seenCRLF ? 0 : 10), manip.getSizeInBytes());
  EXPECT_EQ(288u - (stats.seenCRLF ? 0 : 10), manip.getSizeInCodepoints());
}

TEST(UTF8Tests, ASCIIDrawing) {
  // Open test file
  std::string file_content;
  std::string file_path("lexer/utf8/ascii.txt");
  ASSERT_TRUE(readFileToString(file_path, file_content)) << "Could not open test file \"" << file_path << '"';

  // Prepare string manipulator & other variables
  StringManipulator manip(file_content);

  // Get text statistics
  auto statsResults = getTextStats(manip);
  ASSERT_TRUE(statsResults.hasValue()) 
    << "An error occured while calculating text statistics";
  auto stats = statsResults.getValue();

  // Expected text statistics
  // 18 lines
  // 1190 Characters
  // 847 Spaces
  // 1207 bytes (1990 if LF line endings)
  // 1207 codepoints (1990 if LF line endings)
  EXPECT_EQ(18u, stats.linecount);
  EXPECT_EQ(1173u, stats.charcount);
  EXPECT_EQ(830u, stats.spacecount);
  EXPECT_EQ(1207u - (stats.seenCRLF ? 0 : 17), manip.getSizeInBytes());
  EXPECT_EQ(1207u- (stats.seenCRLF ? 0 : 17), manip.getSizeInCodepoints());
}

TEST(UTF8Tests, Substring) {
  // Open test file : bronze
  std::string bronze_content;
  std::string bronze_path("lexer/utf8/bronzehorseman.txt");
  ASSERT_TRUE(readFileToString(bronze_path, bronze_content)) 
    << "Could not open test file \"" << bronze_path << '"';

  // Open test file : substr
  std::string expected_substr;
  std::string substr_path("lexer/utf8/bronzehorseman.substr.txt");
  ASSERT_TRUE(readFileToString(substr_path, expected_substr)) 
    << "Could not open test file \"" << substr_path << '"';
  StringManipulator::removeBOM(expected_substr);

  // Prepare string manipulator
  StringManipulator manip(bronze_content);

  string_view substr = manip.substring(10, 9);

  EXPECT_EQ(expected_substr, substr) << "Substring was not correct";
}

TEST(UTF8Tests, IndexOfCurCharValidity) {
  // Open test file : bronze
  std::string bronze_content;
  std::string bronze_path("lexer/utf8/bronzehorseman.txt");
  ASSERT_TRUE(readFileToString(bronze_path, bronze_content))
    << "Could not open test file \"" << bronze_path << '"';

  
  // Prepare string manipulator
  StringManipulator manip1,manip2;
  manip1.setStr(bronze_content);
  manip2.setStr(bronze_content);

  for (auto k(0); k < 15; k++)
    manip1.advance();

  manip2.advance(15);

  // test if index is valid
  EXPECT_EQ(15u, manip1.getIndexInCodepoints());
  EXPECT_EQ(manip1.getCurrentChar(), manip2.getCurrentChar());
}