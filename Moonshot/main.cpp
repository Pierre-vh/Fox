#include <iostream>

#include "tests\testers\ExprTester.h"
#include "src\Moonshot\Common\Errors\Errors.h"

// TODO LIST
// Make more detailed error message with a more robust error detection system.

//TODO : BETA
// Remove the enum TT_TYPE competly. Use variant indexes instead.
int main()
{
	auto &err = Moonshot::Errors::getInstance();
	ExprTester *testExpr = new ExprTester();
	err.options.muteLogs = true;
	testExpr->showAST = true;
	std::cout << char(219) << " Expression testing..." << std::endl;

	if (testExpr->run())
		std::cout << char(219) << " All Expr tests finished " << char(175) << "SUCCESS" << std::endl;
	else
		std::cout << char(219) << " All Expr tests finished " << char(175) << "FAILURE" << std::endl;
	delete(testExpr);
	std::cin.get();
	return 0;
}
