////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : UTF8Tests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//	(Unit) Tests for the Lexer's UTF8 String manipulator.
// In short, this tests verifies the capability of the lexer to work on UTF8 strings.
////------------------------------------------------------////

#include "gtest/gtest.h"
#include "TestUtils/TestUtils.hpp"
#include "Moonshot/Fox/Lexer/StringManipulator.hpp"

#include <cwctype>		// std::iswspace

using namespace Moonshot;
using namespace Moonshot::Tests;

/*
	getTextStat : return false if exception happened, and puts e.what() inside exception_details.
	returns true if success and places the results inside linecount, charcount, spacecount.
*/
bool getTextStats(UTF8::StringManipulator &manip, unsigned int& linecount, unsigned int& charcount, unsigned int& spacecount, std::string exception_details = "")
{
	try {
		while (!manip.isAtEndOfStr())
		{
			const auto cur = manip.currentChar();
			if (cur == '\n')
				linecount++;
			else {
				if (std::iswspace(static_cast<wchar_t>(cur)))
					spacecount++;
				charcount++;
			}

			manip.advance();
		}
	}
	catch (std::exception& e)
	{
		exception_details = e.what();
		return false;
	}
	return true;
}

TEST(UTF8Tests,BronzeHorseman)
{
	// Open test file
	std::string file_content;
	std::string file_path("lexer/utf8/bronzehorseman.pushkin.txt");
	ASSERT_TRUE(readFileToString(file_path, file_content)) << "Could not open test file \"" << file_path << '"';

	// Prepare string manipulator & other variables
	UTF8::StringManipulator manip;
	unsigned int linecount = 1, charcount = 0, spacecount = 0;
	manip.setStr(file_content);
	std::string exception_details;

	// Get text statistics
	EXPECT_TRUE(getTextStats(manip, linecount, charcount, spacecount, exception_details)) << "Test failed, exception thrown while iterating through the string. Exception details:" << exception_details;
	
	// Expected text statistics
	// 11 lines
	// 278 Characters
	// 44 Spaces
	EXPECT_EQ(11, linecount) << "Line count incorrect";
	EXPECT_EQ(278, charcount) << "Char count incorrect";
	EXPECT_EQ(44, spacecount) << "Spaces count incorrect";
}

TEST(UTF8Tests, ASCIIDrawing)
{
	// Open test file
	std::string file_content;
	std::string file_path("lexer/utf8/ascii.txt");
	ASSERT_TRUE(readFileToString(file_path, file_content)) << "Could not open test file \"" << file_path << '"';

	// Prepare string manipulator & other variables
	UTF8::StringManipulator manip;
	unsigned int linecount = 1, charcount = 0, spacecount = 0;
	manip.setStr(file_content);
	std::string exception_details;

	// Get text statistics
	EXPECT_TRUE(getTextStats(manip, linecount, charcount, spacecount, exception_details)) << "Test failed, exception thrown while iterating through the string. Exception details:" << exception_details;

	// Expected text statistics
	// 18 lines
	// 1190 Characters
	// 847 Spaces
	EXPECT_EQ(18, linecount) << "Line count incorrect";
	EXPECT_EQ(1190, charcount) << "Char count incorrect";
	EXPECT_EQ(847, spacecount) << "Spaces count incorrect";
}