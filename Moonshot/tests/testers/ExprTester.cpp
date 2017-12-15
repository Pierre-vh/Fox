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
	if (!E_CHECKSTATE)
		return false;

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

	std::cout << "Expression: " << str << std::endl;
	
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
	auto root = p.parseExpr();

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
	root->accept(TypeCheck());

	if (showAST && !shouldFailTC)
		root->accept(Dumper());
	
	if (!shouldFailTC && E_CHECKSTATE)
	{
		std::cout << "[RESULT]:";
		FVal f = root->accept(RTExprVisitor());
		std::cout << dumpFVal(f) << std::endl;
	}

	if (!E_CHECKSTATE && !shouldFailTC)
	{
		std::cout << "\t" << char(192) << "Test failed @ typecheck." << std::endl;
		return false;
	}
	else
		std::cout << "\t" << char(192) << (shouldFailTC ? "Test failed as expected." : "Test passed successfully.") << std::endl;

	root.release();
	root.reset();
	return true;
}
