#include "Tester.h"

using namespace Moonshot;


bool BasicTests::run_lexerMainTests()
{
	// Basic lexer tests.
	Test_CommonUtilities::printTitle(" LEXER ");
	auto test_correct = TesterHelper::readFile(
		util::filepath_MoonshotProj("\\tests\\lexer\\lexer_correct.fox")
	);

	auto test_incorrect = TesterHelper::readFile(
		util::filepath_MoonshotProj("\\tests\\lexer\\lexer_bad.fox")
	);

	std::cout << "Testing lexer with file :\n" << test_correct;
	Lexer l_corr;
	l_corr.lexStr(test_correct);
	if (!E_CHECKSTATE)	return false; // correct tests have failed -> error
	else std::cout << "\n>>NO ERRORS -> OK" << std::endl;

	std::cout << Test_CommonUtilities::generateSpacer();

	std::cout << "Testing lexer with (incorrect) file :\n" << test_incorrect;
	Lexer l_bad;
	l_bad.lexStr(test_incorrect);
	if (E_CHECKSTATE) return false; // failed test have succeeded -> error
	else std::cout << "\n>>ERRORS -> OK" << std::endl;
	// else, ok
	return true;
}

// TESTS
// TO-Do : Find a way to generate a TH_*VIS_VEC easily and clearly
// The solution isn't simple because I need to use emplace_back.
bool Moonshot::BasicTests::run_expressionTests(const bool& printAST)
{
	// Basic expression testing.
	// ASTVisitors needed : TypeCheck, Dumper, RTExprVisitor
	// Parser entry point : parseExpr
	// For parserExpr I need to make a "wrapper" std::function because its signature has a "priority" parameter.

	Test_CommonUtilities::printTitle(" EXPRESSIONS ");

	auto parseExpr_wrapper = [](Parser* p)
	{
		return p->parseExpr();
	};

	auto tests_correct = TesterHelper::readFile_individualLines(
		util::filepath_MoonshotProj("\\tests\\expr\\expr_correct.fox")
	);

	auto tests_incorrect = TesterHelper::readFile_individualLines(
		util::filepath_MoonshotProj("\\tests\\expr\\expr_bad.fox")
	);

	// Prepare vectors
	TH_IVIS_VEC ct_visitors;
	ct_visitors.emplace_back(std::make_unique<TypeCheck>());
	if(printAST)	ct_visitors.emplace_back(std::make_unique<Dumper>());

	TH_RTVIS_VEC rt_visitors;
	rt_visitors.emplace_back(std::make_unique<RTExprVisitor>());

	// Run correct tests
	if (!TesterHelper::standardTest(tests_correct,
		parseExpr_wrapper,
		false,				// Tests must succed
		ct_visitors,
		rt_visitors
		))
			return false;

	// Run incorrect tests
	if (!TesterHelper::standardTest(tests_incorrect,
		parseExpr_wrapper,
		true,				// Tests must fail
		ct_visitors
	))
		return false;

	return true;
}

bool BasicTests::run_expressionStmtTests(const bool& printAST)
{
	// Expression statements tests
	// ASTVisitors needed : TypeCheck, Dumper, RTExprVisitor
	// Parser entry point : parseExprStmt

	Test_CommonUtilities::printTitle("EXPRESSIONS STATEMENTS");


	auto tests_correct = TesterHelper::readFile_individualLines(
		util::filepath_MoonshotProj("\\tests\\exprstmt\\exprstmt_correct.fox")
	);

	auto tests_incorrect = TesterHelper::readFile_individualLines(
		util::filepath_MoonshotProj("\\tests\\exprstmt\\exprstmt_bad.fox")
	);

	// Prepare vectors
	TH_IVIS_VEC ct_visitors;
	ct_visitors.emplace_back(std::make_unique<TypeCheck>());
	if (printAST)	ct_visitors.emplace_back(std::make_unique<Dumper>());

	TH_RTVIS_VEC rt_visitors;
	rt_visitors.emplace_back(std::make_unique<RTExprVisitor>());

	// Run correct tests
	if (!TesterHelper::standardTest(tests_correct,
		&Parser::parseExprStmt,
		false,				// Tests must succed
		ct_visitors,
		rt_visitors
	))
		return false;

	// Run incorrect tests
	if (!TesterHelper::standardTest(tests_incorrect,
		&Parser::parseExprStmt,
		true,				// Tests must fail
		ct_visitors
	))
		return false;

	return true;
}

bool BasicTests::run_varDeclStmtTests(const bool& printAST)
{
	// Var Decl tests
	// ASTVisitors needed :  Dumper
	// Parser entry point : parseVarDeclStmt

	Test_CommonUtilities::printTitle("VAR DECL STATEMENTS");


	auto tests_correct = TesterHelper::readFile_individualLines(
		util::filepath_MoonshotProj("\\tests\\vdecl\\vdecl_correct.fox")
	);

	auto tests_incorrect = TesterHelper::readFile_individualLines(
		util::filepath_MoonshotProj("\\tests\\vdecl\\vdecl_bad.fox")
	);

	// Prepare vectors
	TH_IVIS_VEC ct_visitors;
	if (printAST) ct_visitors.emplace_back(std::make_unique<Dumper>());


	// Run correct tests
	if (!TesterHelper::standardTest(tests_correct,
		&Parser::parseVarDeclStmt,
		false,				// Tests must succed
		ct_visitors
	))
		return false;

	// Run incorrect tests
	if (!TesterHelper::standardTest(tests_incorrect,
		&Parser::parseVarDeclStmt,
		true,				// Tests must fail
		ct_visitors
	))
		return false;

	return true;
}


std::string TesterHelper::readFile(const std::string & fp)
{
	// https://stackoverflow.com/a/2602060/3232822
	std::ifstream t(fp);
	if (!t)
	{
		std::cout << "Failed to open file \"" << fp << "\"." << std::endl;
	}
	std::string str((	std::istreambuf_iterator<char>(t)),
						std::istreambuf_iterator<char>());
	return str;
}

bool TesterHelper::standardTest(const std::vector<std::string>& strs,
	const std::function<std::unique_ptr<Moonshot::IASTNode>(Moonshot::Parser*)>& fn,
	const bool & shouldFail, const TH_IVIS_VEC & ct_vis,
	const TH_RTVIS_VEC & rt_vis)

{
	for (auto& elem : strs)
	{
		if (!standardTest(elem, fn, shouldFail, ct_vis, rt_vis))
			return false;
	}
	return true;
}

std::vector<std::string> TesterHelper::readFile_individualLines(const std::string & fp)
{
	auto fileContent = TesterHelper::readFile(fp);
	std::stringstream stream(fileContent);
	std::vector<std::string> output;

	std::string line;
	while (getline(stream, line))
		output.push_back(line);
	if (output.size() == 0)
		std::cout << "No lines found in file." << std::endl;
	return output;
}

bool TesterHelper::standardTest(const std::string &str,
	const std::function<std::unique_ptr<Moonshot::IASTNode>(Moonshot::Parser*)>& fn, // Signature of parsing functions
	const bool& shouldFail,						// The test should be successful or nah?
	const TH_IVIS_VEC& ct_vis,
	const TH_RTVIS_VEC& rt_vis)
{
	E_RESETSTATE; // Reset state for this test.
	std::cout << Test_CommonUtilities::generateSpacer();
	std::cout << "Testing str : " << str << std::endl;
	Lexer lexr;
	lexr.lexStr(str);
	
	if (!E_CHECKSTATE) return shouldFail;

	Parser parsr(&lexr);
	std::unique_ptr<IASTNode> root = fn(&parsr); // call parser entry point

	if (!E_CHECKSTATE) return shouldFail;

	for (auto& ivis : ct_vis)	// run IVIsitors
	{
		root->accept(*ivis);
		if (!E_CHECKSTATE) return shouldFail;
	}
	for (auto& rtvis : rt_vis)	// run IRTVIsitors
	{
		if (!E_CHECKSTATE) return shouldFail;

		if (dynamic_cast<RTExprVisitor*>(rtvis.get())) // Check if it's a RTExprVis, in that case we dump the result too
		{
			auto exprresult = root->accept(*rtvis);
			if(E_CHECKSTATE)
			std::cout << ">>Expression result: " << fv_util::dumpFVal(exprresult) << std::endl;
		}
		else 
			root->accept(*rtvis);
		if (!E_CHECKSTATE) return shouldFail;
	}

	std::cout << ">>Success" << std::endl;
	return !shouldFail;
}

std::string Test_CommonUtilities::generateSpacer(std::size_t  spacerSize)
{
	std::string rtr;
	for (; spacerSize > 0; spacerSize -= 1)
		rtr += kSpacerChar;
	rtr += '\n';
	return rtr;
}

void Test_CommonUtilities::printTitle(const std::string& title)
{
	std::size_t titleSize = title.size() + title_pad * 2;

	std::string titleSpacer, lr_titleSpacer;

	// generate spacer
	for (int k = 0; k < titleSize; k++)
		titleSpacer += kTitleSpacerChar;

	// generate l&r title spacer
	for (int k = 0; k < title_pad; k++)
		lr_titleSpacer += kTitleSpacerChar;

	std::cout << titleSpacer << std::endl;
	std::cout << lr_titleSpacer << title << lr_titleSpacer << std::endl;
	std::cout << titleSpacer << std::endl;
}
