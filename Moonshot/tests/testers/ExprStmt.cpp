#include "ExprStmt.h"

using namespace Moonshot;
using namespace fv_util;

ExprStmt::ExprStmt()
{
}


ExprStmt::~ExprStmt()
{
}

bool ExprStmt::testStr(const std::string & str, const bool & shouldFailTC)
{
	E_RESETSTATE;

	std::cout << std::endl << "###################" << std::endl << std::endl;
	std::cout << "ExprStmt: " << str << std::endl;

	Lexer l;
	l.lexStr(str);
	l.logAllTokens();
	if (!l.resultSize())
	{
		std::cout << "Result size is 0. It was probably a comment, it was ignored." << std::endl;
		return true;
	}
	if (!E_CHECKSTATE)
	{
		std::cout << "\t" << char(192) << "Test failed @ lexing." << std::endl;
		return shouldFailTC;
	}

	Parser p(&l);
	auto root = p.parseExprStmt();

	if (!E_CHECKSTATE)
	{
		std::cout << "\t" << char(192) << "Test failed @ parsing." << std::endl;
		return shouldFailTC;
	}
	if (!root)
	{
		std::cout << "Root is invalid." << std::endl;
		return shouldFailTC;
	}

	if (showAST && !shouldFailTC)
		root->accept(Dumper());

	TypeCheck tc_vis;
	root->accept(tc_vis);

	if (!E_CHECKSTATE && !shouldFailTC)
	{
		std::cout << "\t" << char(192) << "Test failed @ typecheck." << std::endl;
		return false;
	}

	if (!shouldFailTC && E_CHECKSTATE)
		std::cout << "Typecheck successful, expression returns : " << indexToStr(tc_vis.getReturnTypeOfExpr()) << std::endl;

	std::cout << "\t" << char(192) << (shouldFailTC ? "Test failed as expected." : "Test passed successfully.") << std::endl;

	root.release();
	root.reset();

	return true;
}

bool ExprStmt::run()
{
	std::cout << std::endl << std::endl << "###TEST THAT MUST SUCCEED###" << std::endl << std::endl;
	// run tests that should work successfully
	fp_ = util::filepath_MoonshotProj("\\tests\\exprstmt\\exprstmt_correct.fox");
	if (!testFile(fp_, false))
		return false;
	std::cout << std::endl << std::endl << "###TEST THAT MUST FAIL###" << std::endl << std::endl;
	// run tests that are expected to fail
	fp_ = util::filepath_MoonshotProj("\\tests\\exprstmt\\exprstmt_bad.fox");
	if (!testFile(fp_, true))
		return false;
	return true;
}
