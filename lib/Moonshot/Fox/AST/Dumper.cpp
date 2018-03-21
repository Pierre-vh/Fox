////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Dumper.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Dumper.hpp"
// Exception
#include "Moonshot/Common/Exceptions/Exceptions.hpp"
// type
#include "Moonshot/Common/Types/FoxValueUtils.hpp"
#include "Moonshot/Common/Types/TypesUtils.hpp"
// Include nodes
#include "Moonshot/Fox/AST/ASTExpr.hpp"
#include "Moonshot/Fox/AST/ASTDecl.hpp"
#include "Moonshot/Fox/AST/ASTStmt.hpp"
// utils
#include "Moonshot/Common/Utils/Utils.hpp"
#include "Moonshot/Common/Utils/Utils.hpp" // for enumAsInt
#include <iostream>

using namespace Moonshot;
using namespace TypeUtils;

Dumper::Dumper(const unsigned char & offsetTabs)
{
	offsetTabs_ = offsetTabs;
}

void Dumper::visit(ASTBinaryExpr & node)
{
	std::string op = Util::getFromDict(Dicts::kBinopToStr_dict, node.getOp());
	if (op.size() == 0)
		op = Util::enumAsInt(node.getOp());

	std::cout << getIndent() << "BinaryExpression : Operator " << op;
	// print planned result type if there's one
	if (node.getResultType() != 0 && node.getResultType() != TypeIndex::InvalidIndex)
		std::cout << ", Return type : " << node.getResultType().getTypeName();
	// newline
	std::cout << "\n";

	if (!node.isComplete())
	{
		throw Exceptions::ast_malformation("BinaryExpression node did not have a left and right child.");
		return;
	}
	else
	{
		// PRINT LEFT CHILD
		curindent_++;
		std::cout << getIndent() << "Left child:\n";
		curindent_++;
		node.getLHS()->accept(*this);
		curindent_ -= 2;
		// PRINT RIGHT CHILD
		curindent_++;
		std::cout << getIndent() << "Right child:\n";
		curindent_++;
		node.getRHS()->accept(*this);
		curindent_ -= 2;
	}
	
}

void Dumper::visit(ASTUnaryExpr & node)
{
	std::string op = Util::getFromDict(Dicts::kUnaryOpToStr_dict, node.getOp());
	if (op.size() == 0)
		op = Util::enumAsInt(node.getOp());

	std::cout << getIndent() << "UnaryExpression : Operator " << op;

	if (node.getResultType() != 0 && node.getResultType() != TypeIndex::InvalidIndex)
		std::cout << ", Return type : " << node.getResultType().getTypeName();

	std::cout << "\n";

	curindent_++;
	std::cout << getIndent() << "Child:\n";
	curindent_++;

	if (!node.getChild())
	{
		throw Exceptions::ast_malformation("UnaryExpression node did not have a child.");
		return;
	}

	node.getChild()->accept(*this);
	curindent_ -= 2;
}

void Dumper::visit(ASTCastExpr & node)
{
	std::cout << getIndent() << "CastExpression : Cast Goal:" << node.getCastGoal().getTypeName() << "\n";
	curindent_++;
	std::cout << getIndent() << "Child:\n";
	curindent_++;

	if (!node.getChild())
	{
		throw Exceptions::ast_malformation("CastExpression node did not have a child.");
		return;
	}

	node.getChild()->accept(*this);
	curindent_ -= 2;
}
void Dumper::visit(ASTLiteralExpr & node)
{
	std::cout << getIndent() << "Literal: " << FValUtils::dumpFVal(node.getVal()) << '\n';
}

void Dumper::visit(ASTVarDecl & node)
{
	std::cout << getIndent() << "VarDeclStmt :" << node.getVarAttr().dump() << std::endl;
	if (node.hasInitExpr())
	{
		curindent_ += 1;
		std::cout << getIndent() << "InitExpr\n";
		curindent_ += 1;
		node.getInitExpr()->accept(*this);
		curindent_ -= 2;
	}
}

void Dumper::visit(ASTMemberOfExpr & node)
{
	std::cout << getIndent() << "MemberOf Expr:\n";
	curindent_++;
	std::cout << getIndent() << "Base:\n";
	curindent_++;
	node.getBase()->accept(*this);
	curindent_--;
	std::cout << getIndent() << "Member name:" << node.getMemberNameStr() << "\n";
	curindent_--;
}

void Dumper::visit(ASTDeclRefExpr & node)
{
	std::cout << getIndent() << "DeclRef: name: " << node.getDeclnameStr() << std::endl;
}

void Dumper::visit(ASTFunctionCallExpr & node)
{
	std::cout << getIndent() << "Function Call\n";
	curindent_++;
	std::cout << getIndent() << "Function:\n";

	curindent_++;
	if(node.getDeclRefExpr())
		node.getDeclRefExpr()->accept(*this);
	curindent_--;

	if (node.getExprList()->size())
	{
		std::cout << getIndent() << "Args:\n";
		curindent_++;

		std::size_t count = 0;
		node.getExprList()->iterate([&](auto arg) {
			std::cout << getIndent() << "Arg" << count << '\n';

			curindent_++;
			arg->accept(*this);
			curindent_--;

			count++;
		});

		curindent_--;
	}
	curindent_--;
}

void Dumper::visit(ASTNullStmt& node)
{
	std::cout << getIndent() << "Null Statement\n";
}

void Dumper::visit(ASTFunctionDecl & node)
{
	std::cout << getIndent() << "Function Declaration : name:" << node.getName() << " return type:" << node.getReturnType().getTypeName() << "\n";
	curindent_ += 2;
	std::size_t counter = 0;
	node.iterateArgs([&](auto argdecl){
		std::cout << getIndent() << "Arg" << counter << ":" << argdecl.dump() << std::endl;
		counter += 1;
	});
	curindent_ -= 1;
	std::cout << getIndent() << "Body:" << std::endl;
	curindent_ += 1;
	node.getBody()->accept(*this);
	curindent_ -= 2;
}

void Dumper::visit(ASTReturnStmt & node)
{
	std::cout << getIndent() << "Return statement\n";
	if (node.hasExpr())
	{
		curindent_ += 1;
		node.getExpr()->accept(*this);
		curindent_ -= 1;
	}
}

void Dumper::visit(ASTCompoundStmt & node)
{
	std::cout << getIndent() << "Compound Statement (Contains " << node.size() << " statements)\n";

	curindent_ += 1;

	node.iterateStmts([&](auto stmt) {
		stmt->accept(*this);
	});

	curindent_ -= 1;
}

void Dumper::visit(ASTCondStmt & node)
{
	std::cout << getIndent() << "Condition\n";
	curindent_++;
	// if
	std::cout << getIndent() << "Expression (Condition):\n";
	curindent_++;
	node.getCond()->accept(*this);
	curindent_--;
	std::cout << getIndent() << "Body:\n";
	curindent_++;
	node.getThen()->accept(*this);
	curindent_--;
	// has else?
	if (node.getElse())
	{
		std::cout << getIndent() << "Else:\n";
		curindent_++;
		node.getElse()->accept(*this);
		curindent_--;
	}
	curindent_--;
}

void Dumper::visit(ASTWhileStmt & node)
{
	std::cout << getIndent() << "While Loop\n";

	curindent_++;
	std::cout << getIndent() << "Expression:\n";

	curindent_++;
	node.getCond()->accept(*this);
	curindent_--;

	std::cout << getIndent() << "Body:\n";
	
	curindent_++;
	node.getBody()->accept(*this);
	curindent_--;

	curindent_--;
}

std::string Dumper::getIndent() const
{
	std::string i = getOffsetTabs();
	for (unsigned char k(0); k < curindent_; k++)
		i += "    ";
	if (curindent_ > 1)
		i += u8"┗";
	return i;
}

std::string Moonshot::Dumper::getOffsetTabs() const
{
	std::string i;
	for (unsigned char k(0); k < offsetTabs_; k++)
		i += '\t';
	return i;
}


