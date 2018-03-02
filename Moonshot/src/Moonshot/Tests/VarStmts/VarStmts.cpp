#include "VarStmts.hpp"

#include "Moonshot/Common/DataMap/DataMap.hpp"
#include "Moonshot/Fox/AST/Dumper/Dumper.hpp"

using namespace Moonshot;
using namespace Moonshot::Test;
using namespace Moonshot::Test::TestUtilities;

VarStmts::VarStmts()
{
}


VarStmts::~VarStmts()
{
}

std::string VarStmts::getTestName() const
{
	return "Variable Statements (Declarations & Expressions, including assignements)";
}

bool VarStmts::runTest(Context & context)
{
	// read files
	auto correct_test = readFileToVec(context, "res\\tests\\varstmts\\varstmts_correct.fox");
	auto bad_test = readFileToVec(context, "res\\tests\\varstmts\\varstmts_bad.fox");
	FAILED_RETURN_IF_ERR__SILENT;
	// RUN CORRECT TESTS
	std::cout << std::endl << "Part 1 : Correct tests :\n";
	TypeCheckVisitor tc_good(context); // shared typechecker to keep the datamap

	auto datamap = std::make_shared<DataMap>(context);
	datamap->declareValue(var::varattr("TESTVALUE", TypeIndex::basic_Int, false));

	RTStmtVisitor rt_good(context,datamap);
	for (auto& elem : correct_test)
	{
		std::cout << "\t\xAF Expression :" << elem << std::endl;
		Lexer l(context);
		l.lexStr(elem);
		FAILED_RETURN_IF_ERR("lexing");

		Parser p(context, l.getTokenVector());

		std::unique_ptr<IASTStmt> root;
		if (auto parseres = p.parseStmt())
			root = std::move(parseres.node_);

		FAILED_RETURN_IF_ERR("parsing");

		root->accept(tc_good);
		FAILED_RETURN_IF_ERR("typechecking");

		root->accept(Dumper());

		root->accept(rt_good);
		FAILED_RETURN_IF_ERR("runtime execution");
		context.clearLogs();
	}
	// If all was ok, dump the symbols table
	datamap->dump();
	context.printLogs();
	context.clearLogs();
	// RUN INCORRECT TESTS
	TypeCheckVisitor tc_bad(context);
	RTStmtVisitor rt_bad(context);
	std::cout << std::endl << "Part 2 : Incorrect tests :\n";
	for (auto& elem : bad_test)
	{
		context.clearLogs();
		context.resetState();
		std::cout << "\t\xAF Expression :" << elem << std::endl;
		Lexer l(context);
		l.lexStr(elem);
		SUCCESS_CONTINUE_IF_ERR;

		Parser p(context, l.getTokenVector());

		std::unique_ptr<IASTStmt> root;
		if (auto parseres = p.parseStmt())
			root = std::move(parseres.node_);

		SUCCESS_CONTINUE_IF_ERR;
		SUCCESS_CONTINUE_IF(!root); // fail if root's false

		root->accept(tc_bad);
		SUCCESS_CONTINUE_IF_ERR;

		root->accept(rt_bad);
		SUCCESS_CONTINUE_IF_ERR;

		if (context.isSafe())
		{
			std::cout << "\t\tTest failed (The test was meant to fail, but completed successfully.)\n";
			return false;
		}
	}
	context.resetState();
	return true;
}
