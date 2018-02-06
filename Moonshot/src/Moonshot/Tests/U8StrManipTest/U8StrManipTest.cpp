#include "U8StrManipTest.h"

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
	std::cout << "bronzehorseman.pushkin.txt\n";
	if (!testStr(context,pushkin,11,278,44))
		return false;
	std::cout << "\tSUCCESS\n";
	std::cout << "ascii.txt\n";
	if (!testStr(context, ascii, 18, 1190, (1190-343)))
		return false;
	std::cout << "\tSUCCESS\n";
	return true;
}

bool U8StrManipTest::testStr(Context& context, const std::string& str, unsigned int explinecount, unsigned int expcharcount, unsigned int expspacecount)
{
	unsigned int linecount = 1, charcount = 0, spacecount = 0;

	UTF8StringManipulator manip;

	manip.setStr(str);
	try {
		while (!manip.isAtEndOfStr())
		{
			auto cur = manip.next();
			if (cur == '\n')
				linecount++;
			else {
				if (std::iswspace(cur))
					spacecount++;
				charcount++;
			}
		}
	}
	catch (std::exception& e)
	{
		std::cout << "Test failed, exception thrown while iterating through the string." << std::endl;
		std::cout << e.what() << std::endl;
		return false;
	}
	if (linecount != explinecount)
	{
		std::cout << "Test failed, incorrect linecount. Expected " << explinecount << " lines, found " << linecount << std::endl;
		return false;
	}
	if (charcount != expcharcount) 
	{
		std::cout << "Test failed, incorrect character count, expected " << expcharcount << " char, found " << charcount << std::endl;
		return false;
	}
	if (spacecount != expspacecount) 
	{
		std::cout << "Test failed, incorrect space count, expected " << expspacecount << " spaces, found " << spacecount << std::endl;
		return false;
	}
	return true;
}
