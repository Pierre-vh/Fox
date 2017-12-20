#include <iostream>

#include "tests\testers\ExprTester.h"
#include "tests\testers\VDeclTester.h"
#include "src\Moonshot\Common\Errors\Errors.h"
int main()
{
	auto &err = Moonshot::Errors::getInstance();
	ExprTester *testExpr = new ExprTester();
	err.options.setAll(true);
	testExpr->showAST = false;
	std::cout << char(219) << " Expression testing..." << std::endl;

	if (testExpr->run())
		std::cout << std::endl << char(219) << " All Expr tests finished " << char(175) << "SUCCESS" << std::endl;
	else
		std::cout << std::endl << char(219) << " All Expr tests finished " << char(175) << "FAILURE" << std::endl;

	std::cout << char(219) << " VarDecl testing..." << std::endl;

	E_RESETSTATE;

	VDeclTester *vdecl = new VDeclTester();
	vdecl->showAST = true;
	err.options.setAll(true);

	if (vdecl->run())
		std::cout << std::endl << char(219) << " All VarDecl tests finished " << char(175) << "SUCCESS" << std::endl;
	else
		std::cout << std::endl << char(219) << " All VarDecl tests finished " << char(175) << "FAILURE" << std::endl;

	std::cout << "Finished. Press any key to continue." << std::endl;
	delete(vdecl);
	delete(testExpr);
	std::cin.get();
	return 0;
}
