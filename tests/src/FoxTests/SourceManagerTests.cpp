////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IdentifierTableTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//	(Unit) Tests for the SourceManager.
////------------------------------------------------------////

#include "gtest/gtest.h"
#include "Support/TestUtils.hpp"
#include "Fox/Common/Source.hpp"
#include "Fox/Common/StringManipulator.hpp"

using namespace fox;

TEST(SourceManagerTests, FileIDTests)
{
	EXPECT_FALSE(FileID()) << "Uninitialized File IDs should always be considered invalid!";
}

TEST(SourceManagerTests, LoadingFromFile)
{
	std::string file_path_a = test::convertRelativeTestResPathToAbsolute("lexer/utf8/bronzehorseman.txt");
	std::string file_path_b = test::convertRelativeTestResPathToAbsolute("lexer/utf8/ascii.txt");
	SourceManager srcMgr;
	auto fid_a = srcMgr.loadFromFile(file_path_a);
	auto fid_b = srcMgr.loadFromFile(file_path_b);
	EXPECT_TRUE(fid_a);
	EXPECT_TRUE(fid_b);

	// File path is correct?
	const auto* storeddata_a = srcMgr.getStoredDataForFileID(fid_a);
	const auto* storeddata_b = srcMgr.getStoredDataForFileID(fid_b);

	// File paths are the same? 
	EXPECT_EQ(file_path_a, storeddata_a->fileName);
	EXPECT_EQ(file_path_b, storeddata_b->fileName);

	// Compare contents
	std::string content_a, content_b;
	ASSERT_TRUE(test::readFileToString("lexer/utf8/bronzehorseman.txt", content_a));
	ASSERT_TRUE(test::readFileToString("lexer/utf8/ascii.txt", content_b));

	EXPECT_EQ(content_a, storeddata_a->str);
	EXPECT_EQ(content_b, storeddata_b->str);
}

TEST(SourceManagerTests, LoadingFromString)
{
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

	auto r_str_a = srcMgr.getSourceForFID(fid_a);
	auto r_str_b = srcMgr.getSourceForFID(fid_b);

	// Can we retrieve the correct files?
	EXPECT_EQ(content_a, *r_str_a);
	EXPECT_EQ(content_b, *r_str_b);
}


TEST(SourceManagerTests, SourceRangeTests)
{
	// Create sample source locs
	SourceLoc a(FileID(), 200);
	SourceLoc b(FileID(), 250);

	// Create sample source ranges
	SourceRange ra(a, 50);
	ASSERT_EQ(ra.getOffset(), 50);

	SourceRange rb(a, b);
	ASSERT_EQ(rb.getOffset(), 50);

	// Check if everything is preserved, as expected.
	EXPECT_EQ(rb.getBeginSourceLoc(), a);
	EXPECT_EQ(rb.makeEndSourceLoc(), b);

	EXPECT_EQ(ra.getBeginSourceLoc(), a);
	EXPECT_EQ(ra.makeEndSourceLoc(), b);

	// Another test: only one char sourcelocs
	SourceRange onechar_range_a(a, a);
	SourceRange onechar_range_b(b, b);
	SourceRange onechar_range_c(a);

	EXPECT_TRUE(onechar_range_a.isOnlyOneCharacter());
	EXPECT_TRUE(onechar_range_b.isOnlyOneCharacter());
	EXPECT_TRUE(onechar_range_c.isOnlyOneCharacter());

	EXPECT_EQ(onechar_range_a.getBeginSourceLoc(), onechar_range_a.makeEndSourceLoc());
	EXPECT_EQ(onechar_range_b.getBeginSourceLoc(), onechar_range_b.makeEndSourceLoc());
	EXPECT_EQ(onechar_range_c.getBeginSourceLoc(), onechar_range_c.makeEndSourceLoc());
}

TEST(SourceManagerTests, PreciseLocationTest1)
{
	SourceManager srcMgr;

	std::string fp = test::convertRelativeTestResPathToAbsolute("sourcemanager/precise_test_1.txt");

	// Load file in SourceManager
	auto fid = srcMgr.loadFromFile(fp);
	ASSERT_TRUE(fid) << "File couldn't be loaded in memory";

	// Load file in StringManipulator
	const std::string* ptr = srcMgr.getSourceForFID(fid);
	ASSERT_TRUE(ptr);
	StringManipulator sm(ptr);

	// Loop until we reach the pi sign
	for (; sm.getCurrentChar() != 960 && !sm.eof(); sm.advance());

	if (sm.getCurrentChar() == 960)
	{
		SourceLoc sloc(fid, sm.getIndexInBytes());
		auto result = srcMgr.getCompleteLocForSourceLoc(sloc);

		EXPECT_EQ(result.fileName, fp);
		EXPECT_EQ(result.line, 5);
		EXPECT_EQ(result.column, 6);
	}
	else
	{
		FAIL() << "Couldn't find the pi sign.";
	}
}