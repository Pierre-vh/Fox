#include <iostream>

#include "tests\testers\Tester.h"
#include "src\Moonshot\Common\Exceptions\Exceptions.h"
#include "src\Moonshot\Common\Context\Context.h"

using namespace Moonshot;
/*
	ROADMAP :
		-> Finish the parser and AST, as soon as possible.
		-> Create the last Compile Time visitors needed : 
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
int main()
{
	auto &err = Moonshot::Errors::getInstance();
	bool flag = true;
	
	if (!BasicTests::run_lexerMainTests())
	{
		std::cout << "Test failed" << std::endl;
		flag = false;
	}
	else if (!BasicTests::run_expressionTests(true))
	{
		std::cout << "Test failed" << std::endl;
		flag = false;
	}
	else if (!BasicTests::run_expressionStmtTests(true))
	{
		std::cout << "Test failed" << std::endl;
		flag = false;
	}
	else if (!BasicTests::run_varDeclStmtTests(false))
	{
		std::cout << "Test failed" << std::endl;
		flag = false;
	}
	else if (!StmtTest::runVarStmtTests(false))
	{
		std::cout << "Test failed" << std::endl;
		flag = false;
	}
	Test_CommonUtilities::printTitle(flag ? " SUCCESS " : " FAILURE ");

	// other tests, simply ignore.
	Context ctxt;
	ctxt.reportError("Bad !");
	if (!ctxt.isSafe())
		ctxt.logMessage("Unsafe!");
	ctxt.printLogs();

	try {
		throw Exceptions::lexer_critical_error("TEST");
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	std::cout << "Finished. Press any key to continue." << std::endl;
	std::cin.get();
	return 0;
}
