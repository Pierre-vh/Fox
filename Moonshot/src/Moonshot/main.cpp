#include <iostream>
#include <Windows.h>
#include "Moonshot/Tests/Manager/TestManager.hpp"
using namespace Moonshot;

/*
	TODO :
		Change the ParsingResult class behaviour:
			If the type is a pointer, it uses a std::unique_ptr as type stored
			If the type is not a pointer, it's data type is the same as the type's.
		Implement the rest of the Parsing functions for the functions declarations

		Rework Semantic check classes. See Fox's readme.md
*/

int main()
{

	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 1000);

	std::ios_base::sync_with_stdio(false); // We don't use printf, so we don't need to sync with stdio (CppCoreGuidelines SL.io.10)
	Context context;
	context.optionsManager_.addAttr(OptionsList::exprtest_printAST,true);
	Test::TestManager ts(context);
	ts.addDefaultTests();
	ts.runTests(true);
	std::cout << "Finished. Press any key to continue.\n";
	std::cin.get();
	return 0;
}
