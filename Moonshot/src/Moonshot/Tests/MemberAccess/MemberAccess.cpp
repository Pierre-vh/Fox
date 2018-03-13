////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : MemberAccess.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "MemberAccess.hpp"
#include <sstream>
#include "Moonshot/Fox/AST/Nodes/ASTExpr.hpp"
#include "Moonshot/Fox/AST/Dumper/Dumper.hpp"

using namespace Moonshot;
using namespace Moonshot::Test;
using namespace Moonshot::Test::TestUtilities;

std::string MemberAccess::getTestName() const
{
	return "Member Access";
}

bool MemberAccess::runTest(Context & context)
{
	std::string good = "res\\tests\\memberaccess\\correct.fox";
	std::string bad = "res\\tests\\memberaccess\\bad.fox";

	std::cout << "Correct tests\n";
	auto good_exprs_ = readFileToVec(context, good);
	FAILED_RETURN_IF_ERR__SILENT;
	for (std::size_t k(0); k < good_exprs_.size(); k++)
	{
		std::stringstream ss;
		ss << "line " << k;
		FAILED_RETURN_IF(!testMembAccess(context, good_exprs_[k]),ss.str());
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
		FAILED_RETURN_IF(testMembAccess(context, bad_exprs[k]),ss.str());
		context.printLogs();
		std::cout << "\t\t\xC0Success\n";
	}
	return true;
}

bool MemberAccess::testMembAccess(Context & context, const std::string & str)
{
	std::cout << "str:" << str << std::endl;
	Lexer lex(context);
	lex.lexStr(str);
	FAILED_RETURN_IF_ERR__SILENT;
	Parser parser(context, lex.getTokenVector());

	std::unique_ptr<IASTExpr> node;
	if (auto parseres = parser.parseExpr())
		node = std::move(parseres.result_);

	FAILED_RETURN_IF_ERR__SILENT;
	if (!node) return false;
	node->accept(Dumper());
	return true;
}
