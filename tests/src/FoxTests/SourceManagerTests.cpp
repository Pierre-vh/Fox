////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IdentifierTableTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//	(Unit) Tests for the SourceManager.
////------------------------------------------------------////

#include "gtest/gtest.h"
#include "TestUtils/TestUtils.hpp"

#include "Moonshot/Fox/Common/StringManipulator.hpp"
#include "Moonshot/Fox/Common/Context.hpp"

using namespace Moonshot;

TEST(SourceManagerTests, FileIDTests)
{
	EXPECT_FALSE(FileID()) << "Uninitialized File IDs should always be considered invalid!";
}

TEST(SourceManagerTests, LoadingFromFile)
{
	std::string file_path_a = Tests::convertRelativeTestResPathToAbsolute("lexer/utf8/bronzehorseman.txt");
	std::string file_path_b = Tests::convertRelativeTestResPathToAbsolute("lexer/utf8/ascii.txt");
	Context ctxt;
	auto fid_a = ctxt.sourceManager.loadFromFile(file_path_a);
	auto fid_b = ctxt.sourceManager.loadFromFile(file_path_b);
	EXPECT_TRUE(fid_a);
	EXPECT_TRUE(fid_b);

	// File path is correct?
	auto storeddata_a = ctxt.sourceManager.getFileDataForFID(fid_a);
	auto storeddata_b = ctxt.sourceManager.getFileDataForFID(fid_b);

	// File paths are the same? 
	EXPECT_EQ(file_path_a, storeddata_a->fileName);
	EXPECT_EQ(file_path_b, storeddata_b->fileName);

	// Compare contents
	std::string content_a, content_b;
	ASSERT_TRUE(Tests::readFileToString("lexer/utf8/bronzehorseman.txt", content_a));
	ASSERT_TRUE(Tests::readFileToString("lexer/utf8/ascii.txt", content_b));

	EXPECT_EQ(content_a, storeddata_a->str);
	EXPECT_EQ(content_b, storeddata_b->str);
}

TEST(SourceManagerTests, LoadingFromString)
{
	std::string file_path_a = "lexer/utf8/bronzehorseman.txt";
	std::string file_path_b = "lexer/utf8/ascii.txt";

	std::string content_a, content_b;
	ASSERT_TRUE(Tests::readFileToString(file_path_a, content_a));
	ASSERT_TRUE(Tests::readFileToString(file_path_b, content_b));

	Context ctxt;
	auto fid_a = ctxt.sourceManager.loadFromString(content_a);
	auto fid_b = ctxt.sourceManager.loadFromString(content_b);

	EXPECT_TRUE(fid_a);
	EXPECT_TRUE(fid_b);

	auto r_str_a = ctxt.sourceManager.getSourceForFID(fid_a);
	auto r_str_b = ctxt.sourceManager.getSourceForFID(fid_b);

	// Can we retrieve the correct files?
	EXPECT_EQ(content_a, *r_str_a);
	EXPECT_EQ(content_b, *r_str_b);
}


TEST(SourceManagerTests, SourceRangeTests)
{
	// Create sample source locs
	SourceLoc a(FileID(1), 200);
	SourceLoc b(FileID(1), 250);

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
}

TEST(SourceManagerTests, PreciseLocationTest1)
{
	// Create needed variables
	Context ctxt;

	std::string fp = Tests::convertRelativeTestResPathToAbsolute("sourcelocs/precise_test_1.txt");

	// Load file in SourceManager
	auto fid = ctxt.sourceManager.loadFromFile(fp);
	ASSERT_TRUE(fid) << "File couldn't be loaded in memory";

	// Load file in StringManipulator
	const std::string* ptr = ctxt.sourceManager.getSourceForFID(fid);
	ASSERT_TRUE(ptr);
	StringManipulator sm(ptr);

	// Loop until we reach the pi sign
	for (; sm.getCurrentChar() != 960 && !sm.eof(); sm.advance());

	if (sm.getCurrentChar() == 960)
	{
		SourceLoc sloc(fid, sm.getIndexInBytes());
		auto result = ctxt.sourceManager.getCompleteLocForSourceLoc(sloc);

		EXPECT_EQ(result.fileName, fp);
		EXPECT_EQ(result.character, 960);
		EXPECT_EQ(result.line, 5);
		EXPECT_EQ(result.character_index, 6);
		EXPECT_EQ(result.column, 9);
	}
	else
	{
		FAIL() << "Couldn't find the pi sign.";
	}
}