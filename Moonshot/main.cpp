#include <iostream>

#include "tests\testers\ExprTester.h"
#include "tests\testers\VDeclTester.h"
#include "tests\testers\ExprStmt.h"
#include "src\Moonshot\Common\Errors\Errors.h"
int main()
{
	auto &err = Moonshot::Errors::getInstance();
	ExprTester *testExpr = new ExprTester();
	bool failflag = false;
	err.options.setAll(false);
	testExpr->showAST = true;
	std::cout << char(219) << " Expression testing..." << std::endl;

	if (testExpr->run())
		std::cout << std::endl << char(219) << " All Expr tests finished " << char(175) << "SUCCESS" << std::endl;
	else
		std::cout << std::endl << char(219) << " All Expr tests finished " << char(175) << "FAILURE" << std::endl;

	std::cout << char(219) << " VarDecl testing..." << std::endl;

	if (!E_CHECKSTATE)
		failflag = true;
	E_RESETSTATE;
	
	VDeclTester *vdecl = new VDeclTester();
	vdecl->showAST = false;
	err.options.setAll(true);
	
	if (vdecl->run())
		std::cout << std::endl << char(219) << " All VarDecl tests finished " << char(175) << "SUCCESS" << std::endl;
	else
		std::cout << std::endl << char(219) << " All VarDecl tests finished " << char(175) << "FAILURE" << std::endl;

	std::cout << char(219) << " ExprStmt testing..." << std::endl;

	if (!E_CHECKSTATE)
		failflag = true;
	E_RESETSTATE;
	ExprStmt *estmt = new ExprStmt();
	estmt->showAST = false;
	err.options.setAll(true);

	if (estmt->run())
		std::cout << std::endl << char(219) << " All ExprStmt tests finished " << char(175) << "SUCCESS" << std::endl;
	else
		std::cout << std::endl << char(219) << " All ExprStmt tests finished " << char(175) << "FAILURE" << std::endl;
		
	delete(vdecl);
	delete(testExpr);
	delete(estmt);
	std::cout << "All tests finished : " << (failflag ? "Success." : "Failure.") << std::endl;
	std::cout << "Finished. Press any key to continue." << std::endl;
	std::cin.get();
	return 0;
}
