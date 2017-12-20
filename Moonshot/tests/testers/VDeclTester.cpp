#include "VDeclTester.h"

using namespace Moonshot;
using namespace fv_util;

VDeclTester::VDeclTester()
{
}


VDeclTester::~VDeclTester()
{
}

bool VDeclTester::testStr(const std::string & str, const bool & shouldFailTC)
{
	E_RESETSTATE;

	std::cout << std::endl << "###################" << std::endl << std::endl;
	std::cout << "VarDeclStmt: " << str << std::endl;

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
	auto root = p.parseVarDeclStmt();

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

	if (!shouldFailTC && E_CHECKSTATE)
		std::cout << "\t" << char(192) << "Test successful";
	else 
		std::cout << "\t" << char(192) << "Test failed as expected.";

	root.release();
	root.reset();

	return true;
}

bool VDeclTester::run()
{
	std::cout << std::endl << std::endl << "###TEST THAT MUST SUCCEED###" << std::endl << std::endl;
	// run tests that should work successfully
	fp_ = util::filepath_MoonshotProj("\\tests\\vdecl\\vdecl_correct.fox");
	if (!testFile(fp_, false))
		return false;
	std::cout << std::endl << std::endl << "###TEST THAT MUST FAIL###" << std::endl << std::endl;
	// run tests that are expected to fail
	fp_ = util::filepath_MoonshotProj("\\tests\\vdecl\\vdecl_bad.fox");
	if (!testFile(fp_, true))
		return false;
	return true;
}
