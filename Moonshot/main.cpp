#include <iostream>

#include "tests\testers\Tester.h"

using namespace Moonshot;

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
	std::cout << "Finished. Press any key to continue." << std::endl;
	std::cin.get();
	return 0;
}
