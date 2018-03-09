////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : CompoundStatement.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "CompoundStatements.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTStmt.hpp"
#include "Moonshot/Fox/AST/Dumper/Dumper.hpp"

using namespace Moonshot;
using namespace Moonshot::Test;
using namespace Moonshot::Test::TestUtilities;

CompoundStatements::CompoundStatements()
{
}


CompoundStatements::~CompoundStatements()
{
}

std::string CompoundStatements::getTestName() const
{
	return "Compound Statements Test";
}

bool CompoundStatements::runTest(Context & context)
{
	auto correct_test_1 = readFileToString(context, "res\\tests\\compstmt\\compstmt_correct_1.fox");
	auto correct_test_2 = readFileToString(context, "res\\tests\\compstmt\\compstmt_correct_2.fox");
	auto bad_test_1 = readFileToString(context, "res\\tests\\compstmt\\compstmt_bad_1.fox");
	auto bad_test_2 = readFileToString(context, "res\\tests\\compstmt\\compstmt_bad_2.fox");
	FAILED_RETURN_IF_ERR__SILENT;
	FAILED_RETURN_IF(!parseFile_compoundStatement(context, correct_test_1), "correct_test_1");
	FAILED_RETURN_IF(!parseFile_compoundStatement(context, correct_test_2), "correct_test_2");
	FAILED_RETURN_IF(parseFile_compoundStatement(context, bad_test_1), "bad_test_1");
	FAILED_RETURN_IF(parseFile_compoundStatement(context, bad_test_2), "bad_test_2");
	return true;
}

bool CompoundStatements::parseFile_compoundStatement(Context& context, const std::string & str) const
{
	context.clearLogs();
	context.resetState();
	Lexer lex(context);
	lex.lexStr(str);
	FAILED_RETURN_IF_ERR__SILENT;
	Parser parser(context, lex.getTokenVector());

	std::unique_ptr<ASTCompoundStmt> node;
	if (auto parseres = parser.parseCompoundStatement())
		node = std::move(parseres.result_);

	FAILED_RETURN_IF_ERR__SILENT;
	if (node)
	{
		node->accept(Dumper());
		return true;
	}
	return false;
}
