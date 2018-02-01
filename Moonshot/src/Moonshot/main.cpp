#include <iostream>
#include "Tests/Manager/TestManager.h"
//#include <Windows.h>

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
	About constant folding, I think it's better to do it at the IR level instead of at the AST level. Less messing with pointers and stuff. But it's simpler at the ast level, so I don't know.
	Else, here's how to do it
	-> Constant folding
		-> How to change the ast : Create a typedVisitor<std::unique_ptr<ASTExpr>>
		FOR BINARY EXPR (node.left_ && node.right_)
			->	In the expr function i'll need to call visit on both child at some point. If the result of one of the visit is a valid pointer (they have been changed)
			change the left_ or right_ child to the new ones. DONT FORGET TO FREE THE OLD ONES.
			-> Check if left_ and right_ are literals
				-> true: calculate the expression and return a literal with the result.
				-> false: return nullptr
		FOR UNARY EXPR
			Same stuff, except I'll only care about left_
		-> make getValue private.
		-> updateAST function that takes a unique ptr<astexpr> as argument, if the last value (value_) is a valid pointer, change the argument to that pointer, else, don't change it.
		
		To make the optimization on the whole program, in any node that contains an expression :
			-> call visit on the node
			-> If the result of the visit is nullptr, ignore it, if it's not, std::move the result to the node's expression pointer and free the old expression.
*/
/*
	TO DO : Add other expression tests with another criteria. (Current expression tests checks if the condition evaluates to true, add other ones to analyze expressions better.) (One test for each operator)
*/
int main()
{
	// Currently there's a problem : the windows console stops displaying when you try to print UTF8 chars. find a solution ! :(
	//SetConsoleOutputCP(65001);

	//std::ios_base::sync_with_stdio(false); // We don't use printf, so we don't need to sync with stdio (CppCoreGuidelines SL.io.10)
	Context context;
	context.options.addAttr(OptionsList::exprtest_printAST,false);
	TestManager ts(context);
	ts.addDefaultTests();
	ts.runTests(true);
	std::cout << "Finished. Press any key to continue.\n";
	std::cin.get();
	return 0;
}
