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
		Rework the Condition parsing. Deducing we have a else just based on the presence of a expr_ is just...bad.
		Divide parse else_if into 2 functions, else_if and else. Else returns a IASTStmt, the other returns a condblock. Adapt ParseCondition accordingly.

		Grammar changes todo: (ALL CHANGES MUST BE DONE IN THE PARSER TOO. THE PARSER MUST BE AS CLOSE TO THE GRAMMAR AS POSSIBLE, EVEN IN NAMING)
			remove <eoi> and just use the ';'. I'm probably never going to use another char, so let's not waste space or make the grammar more complex than it needs to be. 
			rename and change rule (the current one is fucked up and doesn't even reflect what the real rule is like..)
				<condition>			= <if> {<elif>} [<else>]
				<cond_if>			= <if_kw>			'(' <expr> ')'	<statement>
				<cond_elif>			= <el_kw> <if_kw>	'(' <expr> ')' 	<statement>
				<cond_else>			= <el_kw>							<statement>

			add the empty statement ';' to <expr_stmt>
					<expr_stmt> = ';' | <expr> ';'

			New grammar version after theses changes : 0.7.0
*/
int main()
{

	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 1000);

	//std::ios_base::sync_with_stdio(false); // We don't use printf, so we don't need to sync with stdio (CppCoreGuidelines SL.io.10)
	Context context;
	context.options.addAttr(OptionsList::exprtest_printAST,true);
	TestManager ts(context);
	ts.addDefaultTests();
	ts.runTests(true);
	std::cout << "Finished. Press any key to continue.\n";
	std::cin.get();
	return 0;
}
