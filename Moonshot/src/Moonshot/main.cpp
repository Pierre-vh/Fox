#include <iostream>
#include <Windows.h>
#include "Moonshot/Tests/Manager/TestManager.hpp"
using namespace Moonshot;
/*
	ROADMAP (sort of):
		-> Finish the parser and AST
		-> Create the last Compile Time visitors needed : 
			-> Update the typechecker to support functions checking
			-> Scope checking
			-> (...) Others if there's a need for them.
		->	Create a Driver class for the frond end.
		->	Design the IR, and create a CodeGen visitor.
			The IR classes should be located in /Common/ (IR Dictionary, generator, etc)
			Nothing from Fox should include files from Badger and vice versa
		-> Start work on Badger.

		Temporary visitor order :
			1. Scope Checking
			2. Type Checking.
			(3. Evaluate Constant Expressions)

		Other ideas
			With constant folding, determinate unreachable code (dead loops, etc)
			Eliminate all statements after a return statement in a function body
			Generally, find ways to simplify the ast as much as possible.
*/
/*
	Needed:
		Rework the parser partially. I need functions to be capable of indicating that :
			* They didn't match the non terminal
			* If they didn't match it, was it because they didn't find it, or because of an error?
		This is needed to avoid flooding the console with useless, non important error messages.

	TODO :
		Implement the system to detect if the parsing of a rule was successful or not to avoid error message flooding. This is a big parser rework and will take a few hours to complete.
		From there, judge if recovery functions can be useful in certain cases. (like in compound statement where the end '}' isn't found-> panic and recover). If yes:
			Add an error counter to the Parser
			When reporting an error, increment err_count
			At the end of the error reporting function, if err_count exceeds a certain thresold, stop parsing (set pos_ to curtok_.size())
			In certain cases, like if a } wasn't found at the end of the compound statement, panic and try to find it to resync. if resync is successful, reset err count. Implement that whenever needed.
		Don't aim for a huge precision error messages, just a good compromise.
*/

int main()
{

	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 1000);

	std::ios_base::sync_with_stdio(false); // We don't use printf, so we don't need to sync with stdio (CppCoreGuidelines SL.io.10)
	Context context;
	context.optionsManager_.addAttr(OptionsList::exprtest_printAST,true);
	TestManager ts(context);
	ts.addDefaultTests();
	ts.runTests(true);
	std::cout << "Finished. Press any key to continue.\n";
	std::cin.get();
	return 0;
}
