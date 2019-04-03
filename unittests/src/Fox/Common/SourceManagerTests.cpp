//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : IdentifierTableTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  (Unit) Tests for the SourceManager.
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Support/TestUtils.hpp"
#include "Fox/Common/SourceManager.hpp"
#include "utfcpp/utf8.hpp"

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
  EXPECT_EQ(aPath, srcMgr.getFileName(aFile));
  EXPECT_EQ(bPath, srcMgr.getFileName(bFile));

  // The content is correctly stored
  std::string content_a, content_b;
  ASSERT_TRUE(test::readFileToString("lexer/utf8/bronzehorseman.txt", content_a));
  ASSERT_TRUE(test::readFileToString("lexer/utf8/ascii.txt", content_b));
  EXPECT_EQ(content_a, srcMgr.getFileContent(aFile));
  EXPECT_EQ(content_b, srcMgr.getFileContent(bFile));
}

TEST(SourceManagerTest, LoadingFromString) {
  std::string file_path_a = "lexer/utf8/bronzehorseman.txt";
  std::string file_path_b = "lexer/utf8/ascii.txt";

  std::string content_a, content_b;
  ASSERT_TRUE(test::readFileToString(file_path_a, content_a));
  ASSERT_TRUE(test::readFileToString(file_path_b, content_b));

  SourceManager srcMgr;
  auto fid_a = srcMgr.loadFromString(content_a, "a");
  auto fid_b = srcMgr.loadFromString(content_b, "b");

  EXPECT_TRUE(fid_a);
  EXPECT_TRUE(fid_b);

  string_view r_str_a = srcMgr.getFileContent(fid_a);
  string_view r_str_b = srcMgr.getFileContent(fid_b);

  EXPECT_EQ(content_a, r_str_a);
  EXPECT_EQ(content_b, r_str_b);
}


TEST(SourceManagerTest, SourceRange) {
  // Create sample source locs
  SourceLoc a(FileID(), 200);
  SourceLoc b(FileID(), 250);

  // Create sample source ranges
  SourceRange ra(a, 50);
  ASSERT_EQ(ra.getRawOffset(), 50u);

  SourceRange rb(a, b);
  ASSERT_EQ(rb.getRawOffset(), 50u);

  // Check if everything is preserved, as expected.
  EXPECT_EQ(rb.getBeginLoc(), a);
  EXPECT_EQ(rb.getEndLoc(), b);

  EXPECT_EQ(ra.getBeginLoc(), a);
  EXPECT_EQ(ra.getEndLoc(), b);

  // Another test: only one char sourcelocs
  SourceRange onechar_range_a(a, a);
  SourceRange onechar_range_b(b, b);
  SourceRange onechar_range_c(a);

  EXPECT_EQ(onechar_range_a.getBeginLoc(), onechar_range_a.getEndLoc());
  EXPECT_EQ(onechar_range_b.getBeginLoc(), onechar_range_b.getEndLoc());
  EXPECT_EQ(onechar_range_c.getBeginLoc(), onechar_range_c.getEndLoc());
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
  string_view str = srcMgr.getFileContent(testFile);
  auto it = str.begin();
  auto end = str.end();

  // Loop until we reach the pi sign
  for (; (utf8::peek_next(it, end) != 960u) 
    && (it != str.end()); utf8::next(it, end));

  ASSERT_EQ(utf8::peek_next(it, end), 960u) << "Couldn't find the PI sign";

  SourceLoc sloc(testFile, std::distance(str.begin(), it));
  auto completeLoc = srcMgr.getCompleteLoc(sloc);

  EXPECT_EQ(completeLoc.fileName, testFilePath);
  EXPECT_EQ(completeLoc.line, 5u);
  EXPECT_EQ(completeLoc.column, 7u);
}

TEST(SourceManagerTest, CompleteLocToString) {
  std::string testFilePath = test::getPath("sourcemanager/precise_test_1.txt");
  SourceManager srcMgr;
  auto result = srcMgr.readFile(testFilePath);
  FileID testFile = result.first;
  ASSERT_TRUE(testFile) << "Error while reading '" << testFilePath 
    << "': " << toString(result.second);

  SourceLoc loc_a(testFile);
  CompleteLoc cloc_a = srcMgr.getCompleteLoc(loc_a);
  EXPECT_EQ(cloc_a.toString(/*printFileName*/ false), "1:1");
  EXPECT_EQ(cloc_a.toString(/*printFileName*/ true), testFilePath + ":1:1");

  SourceLoc loc_b(testFile, 10);
  CompleteLoc cloc_b = srcMgr.getCompleteLoc(loc_b);
  EXPECT_EQ(cloc_b.toString(/*printFileName*/ false), "1:11");
  EXPECT_EQ(cloc_b.toString(/*printFileName*/ true), testFilePath + ":1:11");
}

bool isCRLF(string_view str) {
  for (std::size_t k = 0, sz = str.size(); k < sz; ++k) {
    char cur = str[k];
    char prev = (k > 0) ? str[k-1] : 0; // use 0 if k is 0
    if(cur == '\n')
      return prev == '\r';
  }
  return false;
}

TEST(SourceManagerTest, CompleteRangeToString) {
  std::string testFilePath = test::getPath("sourcemanager/precise_test_1.txt");
  SourceManager srcMgr;
  auto result = srcMgr.readFile(testFilePath);
  FileID testFile = result.first;
  ASSERT_TRUE(testFile) << "Error while reading '" << testFilePath 
    << "': " << toString(result.second);
  ASSERT_TRUE(testFile) << "File couldn't be read";

  bool crlf = isCRLF(srcMgr.getFileContent(testFile));

  SourceRange r_a(SourceLoc(testFile), 10);
  CompleteRange cr_a = srcMgr.getCompleteRange(r_a);
  EXPECT_EQ(cr_a.toString(/*printFileName*/ false), "1:1-1:11");
  EXPECT_EQ(cr_a.toString(/*printFileName*/ true),  testFilePath + ":1:1-1:11");

  // Here, we want to land at 3-2, to achieve that with CRLF, we must 
  // end the range at loc+5, but with LF, we must end the range at +3.
  SourceRange r_b(SourceLoc(testFile, 14), crlf ? 5 : 3);
  CompleteRange cr_b = srcMgr.getCompleteRange(r_b);
  EXPECT_EQ(cr_b.toString(/*printFileName*/ false), "1:15-3:2");
  EXPECT_EQ(cr_b.toString(/*printFileName*/ true), testFilePath + ":1:15-3:2");

  SourceRange r_c(SourceLoc(testFile, 5));
  CompleteRange cr_c = srcMgr.getCompleteRange(r_c);
  EXPECT_EQ(cr_c.toString(/*printFileName*/ false), "1:6-1:6");
  EXPECT_EQ(cr_c.toString(/*printFileName*/ true), testFilePath + ":1:6-1:6");

}