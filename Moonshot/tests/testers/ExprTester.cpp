#include "ExprTester.h"

using namespace Moonshot;

ExprTester::ExprTester()
{
}


ExprTester::~ExprTester()
{
}

bool ExprTester::run()
{
	// run tests that should work successfully
	fp_ = util::filepath_MoonshotProj("\\tests\\expr\\expr_correct.fox");
	if (!testFile(fp_, false))
		return false;
	// run tests that are expected to fail
	fp_ = util::filepath_MoonshotProj("\\tests\\expr\\expr_bad.fox");
	if (!testFile(fp_, true))
		return false;
	return true;
}

bool ExprTester::testFile(const std::string & fp, const bool & shouldFail)
{
	std::ifstream file(fp); // Open file
	if (!file)
		std::cout << "Couldn't open file " << fp_ << std::endl;
	std::string line;
	while (std::getline(file, line))
	{
		if (!testStr(line, shouldFail))
			return false;
	}
	return true;
}

bool ExprTester::testStr(const std::string & str, const bool &shouldFailTC)
{
	E_RESETSTATE

	std::cout << std::endl << "Expression: " << str << std::endl;
	
	Lexer l;
	l.lexStr(str);

	if (!E_CHECKSTATE)
	{
		std::cout << "\t" << char(192) << "Test failed @ lexing." << std::endl;
		return false;
	}

	Parser p(&l);
	auto root = p.parseExpr();

	if (!E_CHECKSTATE)
	{
		std::cout << "\t" << char(192) << "Test failed @ parsing." << std::endl;
		return false;
	}
	TypeCheck *tc_check = new TypeCheck();
	root->accept(tc_check);

	if (!E_CHECKSTATE && !shouldFailTC)
	{
		std::cout << "\t" << char(192) << "Test failed @ typecheck." << std::endl;
		return false;
	}
	else
		std::cout << "\t" << char(192) << (shouldFailTC ? "Test failed as expected." : "Test passed successfully.") << std::endl;

	if (showAST && !shouldFailTC)
		root->accept(new Dumper());

	std::cout << "result" << std::endl;
	
	if (!shouldFailTC && E_CHECKSTATE)
	{
		RTExprVisitor *v = new RTExprVisitor();
		FVal f = root->accept(v);
		std::cout << dumpFVal(f);
	}
	root.release();
	delete(tc_check);

	return true;
}
