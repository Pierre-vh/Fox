#include <iostream>

#include "tests\testers\ExprTester.h"
#include "src\Moonshot\Common\Errors\Errors.h"

// TODO : change operations return type in typecheck.
// Divisions should return float
// Polish the overall expr evaluator
int main()
{
	auto err = Moonshot::Errors::getInstance();
	ExprTester *testExpr = new ExprTester();
	err->options.muteLogs = true;
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
