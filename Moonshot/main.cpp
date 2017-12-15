#include <iostream>

#include "tests\testers\ExprTester.h"
#include "src\Moonshot\Common\Errors\Errors.h"
int main()
{
	auto &err = Moonshot::Errors::getInstance();
	ExprTester *testExpr = new ExprTester();
	err.options.setAll(false);
	testExpr->showAST = false;
	std::cout << char(219) << " Expression testing..." << std::endl;

	if (testExpr->run())
		std::cout << char(219) << " All Expr tests finished " << char(175) << "SUCCESS" << std::endl;
	else
		std::cout << char(219) << " All Expr tests finished " << char(175) << "FAILURE" << std::endl;
	delete(testExpr);
	std::cin.get();
	return 0;
}
