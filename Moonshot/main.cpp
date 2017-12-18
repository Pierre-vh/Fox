#include <iostream>

#include "tests\testers\ExprTester.h"
#include "src\Moonshot\Common\Errors\Errors.h"
int main()
{
	auto &err = Moonshot::Errors::getInstance();
	ExprTester *testExpr = new ExprTester();
	err.options.setAll(true);
	testExpr->showAST = false;
	std::cout << char(219) << " Expression testing..." << std::endl;

	if (testExpr->run())
		std::cout << char(219) << " All Expr tests finished " << char(175) << "SUCCESS" << std::endl;
	else
		std::cout << char(219) << " All Expr tests finished " << char(175) << "FAILURE" << std::endl;
	
	std::cout << "Testing dumpFval on varattr" << std::endl;
	Moonshot::var::varattr testvattr("foo", Moonshot::fv_util::fval_float, true);
	FVal testfval(testvattr);
	std::cout << Moonshot::fv_util::dumpFVal(testfval) << std::endl;

	std::cout << "Finished. Press any key to continue." << std::endl;
	delete(testExpr);
	std::cin.get();
	return 0;
}
