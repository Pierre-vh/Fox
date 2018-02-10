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

		Other ideas
			With constant folding, determinate unreachable code (dead loops, etc)
			Eliminate all statements after a return statement in a function body
			Generally, find ways to simplify the ast as much as possible.
*/
/*
	TODO:
		Rework the parser partially. I need functions to be capable of indicating that :
			* They didn't match the non terminal
			* If they didn't match it, was it because they didn't find it, or because of an error?
		This is needed to avoid flooding the console with useless, non important error messages.

		Also, generate better error messages, and make the parser able to recover from unexpected token (with a limited number of retries).	
		Functions : TryMatchToken() (tries once and return false if not) Require(<bool(lambda_function)>)  (retries the function x times before giving up with error message)
			Example usage : Require({return TryMatchToken(sign::P_SEMICOLON);});
			But this kind of looks ugly, maybe find a better system that's also flexible, and avoid function spamming like TryMatchLiteral,TryMatchID,RequireMatchLIteral,RequireMatchId,etc


		Example of good error message
		let foo : int = 3+3/*4);
		"Unexpected token "*" at line 1"
		"Unexpected token ")" at line 1"
			Note that the parsing continued, and indicated further error messages, which is pretty useful to avoid recompiling a lot of time to fix every error.
			That will be the main focus for the rework.

		Rework goals:
			* Better error messages, more user friendly : stop flooding, give more info.
			* Parsing should continue after a unexpected token has been found, unless they can't find the desired token after X tentatives
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
