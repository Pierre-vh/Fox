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
	TODO :
		Change the ParsingResult class behaviour:
			If the type is a pointer, it uses a std::unique_ptr as type stored
			If the type is not a pointer, it's data type is the same as the type's.

		Improve the way the type is stored, add a "type" object which encapsulate an index. The type's object behaviour can be
		changed if we need to store a user-defined type. The parser/typechecker should never pass std::size_t, just Type objects, and operate on them.

		Implement the rest of the Parsing functions for the functions declarations

		Finish to implement the parser + ast: functions + import (add it back to the grammar)
		Add scope checks, finish typechecker
		TAC IR (w/ Basic block detection, constant prop/fold at first) (this is going to be fun, kek)
		TAC IR -> Bytecode Generation phase
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
