////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : FuncDecl.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "FunctionCalls.hpp"
#include <sstream>
#include "Moonshot/Fox/AST/Nodes/ASTExpr.hpp"
#include "Moonshot/Fox/AST/Dumper/Dumper.hpp"

using namespace Moonshot;
using namespace Moonshot::Test;
using namespace Moonshot::Test::TestUtilities;

FunctionCalls::~FunctionCalls()
{
}

std::string FunctionCalls::getTestName() const
{
	return "Function Declarations";
}

bool FunctionCalls::runTest(Context & context)
{
	std::string good = "res\\tests\\declcall_fn\\correct.fox";
	std::string bad = "res\\tests\\declcall_fn\\bad.fox";

	std::cout << "Correct tests\n";
	auto good_exprs_ = readFileToVec(context, good);
	FAILED_RETURN_IF_ERR__SILENT;
	for (std::size_t k(0); k < good_exprs_.size(); k++)
	{
		std::stringstream ss;
		ss << "line " << k;
		FAILED_RETURN_IF(!testFuncCall(context, good_exprs_[k]),ss.str());
		std::cout << "\t\t\xC0Success\n";
	}

	std::cout << "Bad tests\n";
	auto bad_exprs = readFileToVec(context, bad);
	FAILED_RETURN_IF_ERR__SILENT;
	for (std::size_t k(0); k < bad_exprs.size(); k++)
	{
		context.clearLogs();
		context.resetState();

		std::stringstream ss;
		ss << "line " << k;
		FAILED_RETURN_IF(testFuncCall(context, bad_exprs[k]),ss.str());
		context.printLogs();
		std::cout << "\t\t\xC0Success\n";
	}
	return true;
}

bool FunctionCalls::testFuncCall(Context & context, const std::string & str)
{
	Lexer lex(context);
	lex.lexStr(str);
	FAILED_RETURN_IF_ERR__SILENT;
	Parser parser(context, lex.getTokenVector());

	std::unique_ptr<IASTExpr> node;
	if (auto parseres = parser.parseDeclCall())
		node = std::move(parseres.result_);

	FAILED_RETURN_IF_ERR__SILENT;
	if (!dynamic_cast<ASTFunctionCallExpr*>(node.get()))
	{
		context.logMessage("Expr was not recognized as a function call.");
		return false;
	}
	else
	{
		if (!node) return false;
		node->accept(Dumper());
		return true;
	}
}
