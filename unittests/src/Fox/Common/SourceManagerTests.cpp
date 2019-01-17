//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : IdentifierTableTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  (Unit) Tests for the SourceManager.
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Support/TestUtils.hpp"
#include "Fox/Common/Source.hpp"
#include "Fox/Common/StringManipulator.hpp"

using namespace fox;

TEST(SourceManagerTest, FileIDTests) {
  EXPECT_FALSE(FileID()) << "Uninitialized File IDs should always be considered invalid!";
}

TEST(SourceManagerTest, LoadingFromFile) {
  std::string aPath = test::getPath("lexer/utf8/bronzehorseman.txt");
  std::string bPath = test::getPath("lexer/utf8/ascii.txt");
  SourceManager srcMgr;
  auto aRes = srcMgr.readFile(aPath);
  auto bRes = srcMgr.readFile(bPath);
  FileID aFile = aRes.first;
  FileID bFile = bRes.first;
  EXPECT_TRUE(aFile) << "Error while reading '" << aPath 
    << "': " << toString(aRes.second); 
  EXPECT_TRUE(bFile) << "Error while reading '" << bPath 
    << "': " << toString(bRes.second); 

  // The name is correctly stored
  EXPECT_EQ(aPath, srcMgr.getSourceName(aFile));
  EXPECT_EQ(bPath, srcMgr.getSourceName(bFile));

  // The content is correctly stored
  std::string content_a, content_b;
  ASSERT_TRUE(test::readFileToString("lexer/utf8/bronzehorseman.txt", content_a));
  ASSERT_TRUE(test::readFileToString("lexer/utf8/ascii.txt", content_b));
  EXPECT_EQ(content_a, srcMgr.getSourceStr(aFile));
  EXPECT_EQ(content_b, srcMgr.getSourceStr(bFile));
}

TEST(SourceManagerTest, LoadingFromString) {
  std::string file_path_a = "lexer/utf8/bronzehorseman.txt";
  std::string file_path_b = "lexer/utf8/ascii.txt";

  std::string content_a, content_b;
  ASSERT_TRUE(test::readFileToString(file_path_a, content_a));
  ASSERT_TRUE(test::readFileToString(file_path_b, content_b));

  SourceManager srcMgr;
  auto fid_a = srcMgr.loadFromString(content_a);
  auto fid_b = srcMgr.loadFromString(content_b);

  EXPECT_TRUE(fid_a);
  EXPECT_TRUE(fid_b);

  string_view r_str_a = srcMgr.getSourceStr(fid_a);
  string_view r_str_b = srcMgr.getSourceStr(fid_b);

  EXPECT_EQ(content_a, r_str_a);
  EXPECT_EQ(content_b, r_str_b);
}


TEST(SourceManagerTest, SourceRange) {
  // Create sample source locs
  SourceLoc a(FileID(), 200);
  SourceLoc b(FileID(), 250);

  // Create sample source ranges
  SourceRange ra(a, 50);
  ASSERT_EQ(ra.getRawOffset(), 50);

  SourceRange rb(a, b);
  ASSERT_EQ(rb.getRawOffset(), 50);

  // Check if everything is preserved, as expected.
  EXPECT_EQ(rb.getBegin(), a);
  EXPECT_EQ(rb.getEnd(), b);

  EXPECT_EQ(ra.getBegin(), a);
  EXPECT_EQ(ra.getEnd(), b);

  // Another test: only one char sourcelocs
  SourceRange onechar_range_a(a, a);
  SourceRange onechar_range_b(b, b);
  SourceRange onechar_range_c(a);

  EXPECT_TRUE(onechar_range_a.isOnlyOneCharacter());
  EXPECT_TRUE(onechar_range_b.isOnlyOneCharacter());
  EXPECT_TRUE(onechar_range_c.isOnlyOneCharacter());

  EXPECT_EQ(onechar_range_a.getBegin(), onechar_range_a.getEnd());
  EXPECT_EQ(onechar_range_b.getBegin(), onechar_range_b.getEnd());
  EXPECT_EQ(onechar_range_c.getBegin(), onechar_range_c.getEnd());
}

TEST(SourceManagerTest, PreciseLocation) {
  SourceManager srcMgr;

  std::string testFilePath = test::getPath("sourcemanager/precise_test_1.txt");

  // Load file in SourceManager
  auto result = srcMgr.readFile(testFilePath);
  FileID testFile = result.first;
  ASSERT_TRUE(testFile) << "Error while reading '" << testFilePath 
    << "': " << toString(result.second);

  // Load file in StringManipulator
  string_view ptr = srcMgr.getSourceStr(testFile);
  StringManipulator sm(ptr);

  // Loop until we reach the pi sign
  for (; sm.getCurrentChar() != 960 && !sm.eof(); sm.advance());

  if (sm.getCurrentChar() == 960) {
    SourceLoc sloc(testFile, sm.getIndexInBytes());
    auto result = srcMgr.getCompleteLoc(sloc);

    EXPECT_EQ(result.fileName, testFilePath);
    EXPECT_EQ(result.line, 5);
    EXPECT_EQ(result.column, 7);
  }
  else {
    FAIL() << "Couldn't find the pi sign.";
  }
}

TEST(SourceManagerTest, SourceLocToString) {
  std::string testFilePath = test::getPath("sourcemanager/precise_test_1.txt");
  SourceManager srcMgr;
  auto result = srcMgr.readFile(testFilePath);
  FileID testFile = result.first;
  ASSERT_TRUE(testFile) << "Error while reading '" << testFilePath 
    << "': " << toString(result.second);
  SourceLoc loc_a(testFile);
  SourceLoc loc_b(testFile, 10);
  EXPECT_EQ(loc_a.toString(srcMgr), "1:1");
  EXPECT_EQ(loc_b.toString(srcMgr), "1:11");
}

TEST(SourceManagerTest, SourceRangeToString) {
  std::string testFilePath = test::getPath("sourcemanager/precise_test_1.txt");
  SourceManager srcMgr;
  auto result = srcMgr.readFile(testFilePath);
  FileID testFile = result.first;
  ASSERT_TRUE(testFile) << "Error while reading '" << testFilePath 
    << "': " << toString(result.second);
  ASSERT_TRUE(testFile) << "File couldn't be read";
  SourceRange r_a(SourceLoc(testFile), 10);
  SourceRange r_b(SourceLoc(testFile, 14), 3);
  SourceRange r_c(SourceLoc(testFile, 5));

  EXPECT_EQ(r_a.toString(srcMgr), "1:1-11");
  EXPECT_EQ(r_b.toString(srcMgr), "1:15-2:2");
  EXPECT_EQ(r_c.toString(srcMgr), "1:6");
}