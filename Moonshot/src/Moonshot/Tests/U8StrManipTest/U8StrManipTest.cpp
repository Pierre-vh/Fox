#include "U8StrManipTest.hpp"

#include "Moonshot/Common/UTF8/StringManipulator.hpp"
#include <cwctype>		// std::iswspace
#include <sstream>

using namespace Moonshot;
using namespace TestUtilities;

U8StrManipTest::~U8StrManipTest()
{

}

std::string U8StrManipTest::getTestName() const
{
	return "UTF8 Test";
}

bool U8StrManipTest::runTest(Context & context)
{
	std::string pushkin = readFileToString(context, "res\\tests\\utf8\\bronzehorseman.pushkin.txt");
	std::string ascii = readFileToString(context, "res\\tests\\utf8\\ascii.txt");
	if (!context.isSafe())
		return false;
	// charcount & spacecount from https://charcounter.com/en/
	std::cout << ">bronzehorseman.pushkin.txt";
	if (!testStr(context,pushkin,11,278,44))
		return false;
	std::cout << "\tSUCCESS\n";
	std::cout << ">ascii.txt";
	if (!testStr(context, ascii, 18, 1190, (1190-343)))
		return false;
	std::cout << "\tSUCCESS\n";

	std::cout << ">Append test:";
	{
		const char32_t u8ch = L'ш';
		std::string str = "foo";
		UTF8::append(str, u8ch);
		str += "bar";
		if (str != u8"fooшbar")
		{
			std::cout << "\tFailed to append, or append function doesn't work as intended.\n";
			return false;
		}
		std::cout << "\tSUCCESS\n";
	}

	std::cout << ">Substr test:";
	{
		std::string str(u8"﻿Τη γλώσσα μου έδωσαν ελληνική");
		std::string exp_sub(u8"γλώσσα μου");
		UTF8::StringManipulator manip;
		manip.setStr(str);
		std::string sub = manip.substring(3,10);
		if (sub != exp_sub)
		{
			std::cout << "\tFailed. Substring was different from expected substring.\n\tExpected substring :\"" << exp_sub << "\", substring returned:\"" << sub << "\"\n";
			std::cout << "Expected substr size in bytes:" << exp_sub.size() << " Substr size in byte:" << sub.size() << std::endl;
			return false;
		}
		std::cout << "\tSUCCESS\n";
	}

	std::cout << ">getchar test:";
	{
		std::string str(u8"﻿Τη γλώσσα μου έδωσαν ελληνική");
		UTF8::StringManipulator manip;
		manip.setStr(str);
		if (L'ώ' != manip.getChar(5))
		{
			std::cout << "\t Failed, char returned was different from the one expected." << std::endl;
			return false;
		}
		std::cout << "\tSUCCESS\n";
	}
	return true;
}

bool U8StrManipTest::testStr(Context& context, const std::string& str, unsigned int explinecount, unsigned int expcharcount, unsigned int expspacecount)
{
	unsigned int linecount = 1, charcount = 0, spacecount = 0;

	UTF8::StringManipulator manip;

	manip.setStr(str);
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
		std::stringstream out;
		out << "Test failed, exception thrown while iterating through the string." << "\n";
		out << e.what() << "\n";
		context.reportError(out.str());
		return false;
	}
	if (linecount != explinecount)
	{
		std::stringstream out;
		out << "Test failed, incorrect linecount. Expected " << explinecount << " lines, found " << linecount << "\n";
		context.reportError(out.str());
	}
	if (charcount != expcharcount) 
	{
		std::stringstream out;
		out << "Test failed, incorrect character count, expected " << expcharcount << " char, found " << charcount << "\n";
		context.reportError(out.str());
	}
	if (spacecount != expspacecount) 
	{
		std::stringstream out;
		out << "Test failed, incorrect space count, expected " << expspacecount << " spaces, found " << spacecount << "\n";
		context.reportError(out.str());
	}
	return true;
}
