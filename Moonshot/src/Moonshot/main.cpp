#include <iostream>

#include "Tests/Manager/TestManager.h"

#include <Windows.h>
using namespace Moonshot;
/*
	ROADMAP :
		-> Finish the parser and AST, as soon as possible.
		-> Create the last Compile Time visitors needed : 
			-> Update the typechecker to support functions checking
			-> Scope checking (using the tree)
			-> (...) Others if there's a need for them.
		->	Create a "Front-End" class for Fox for fox, that
			takes a string or a file as input and runs the Lexer,Parser and Visitors needed.
			Do not make the class fully opaque, with just a function taking the str and outputting the std::vector<Instruction>
			But create functions like : readFile(),readStr(),runCompileTimeChecks(),generateIR(),..
		->	Design the IR, and create a CodeGen visitor.
			The IR class should be located in /Common/
			Nothing from Fox should include files from Badger.
		-> Start work on Badger.

		Temporary visitor order :
			1. Scope Checking
			2. Type Checking.
			(3. Evaluate Constant Expressions)
*/
/*
	Known bugs : Large int's arent recognized by the Token id function. Might revert to a regex based one!
*/
/*
	TO DO : Add other expression tests with another criteria. (Current expression tests checks if the condition evaluates to true, add other ones to analyze expressions better.) (One test for each operator)
*/
int main()
{
	SetConsoleOutputCP(CP_UTF8);

	Context context;
	TestManager ts(context);
	ts.addDefaultTests();
	ts.runTests(true);
	std::cout << "Finished. Press any key to continue." << std::endl;
	std::cin.get();
	return 0;
}
