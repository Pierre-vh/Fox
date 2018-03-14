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
#include "Moonshot/Fox/AST/Nodes/ASTExpr.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTDecl.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTStmt.hpp"
// utils
#include "Moonshot/Common/Utils/Utils.hpp"
#include "Moonshot/Common/Utils/Utils.hpp" // for enumAsInt
#include <iostream>

using namespace Moonshot;
using namespace TypeUtils;

Dumper::~Dumper()
{
}

void Dumper::visit(ASTBinaryExpr & node)
{
	std::string op = Util::getFromDict(Dicts::kBinopToStr_dict, node.getOp());
	if (op.size() == 0)
		op = Util::enumAsInt(node.getOp());

	std::cout << tabs() << "BinaryExpression : Operator " << op;
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
		tabcount_++;
		std::cout << tabs() << "Left child:\n";
		tabcount_++;
		node.getLHS()->accept(*this);
		tabcount_ -= 2;
		// PRINT RIGHT CHILD
		tabcount_++;
		std::cout << tabs() << "Right child:\n";
		tabcount_++;
		node.getRHS()->accept(*this);
		tabcount_ -= 2;
	}
	
}

void Dumper::visit(ASTUnaryExpr & node)
{
	std::string op = Util::getFromDict(Dicts::kUnaryOpToStr_dict, node.getOp());
	if (op.size() == 0)
		op = Util::enumAsInt(node.getOp());

	std::cout << tabs() << "UnaryExpression : Operator " << op;

	if (node.getResultType() != 0 && node.getResultType() != TypeIndex::InvalidIndex)
		std::cout << ", Return type : " << node.getResultType().getTypeName();

	std::cout << "\n";

	tabcount_++;
	std::cout << tabs() << "Child:\n";
	tabcount_++;

	if (!node.getChild())
	{
		throw Exceptions::ast_malformation("UnaryExpression node did not have a child.");
		return;
	}

	node.getChild()->accept(*this);
	tabcount_ -= 2;
}

void Dumper::visit(ASTCastExpr & node)
{
	std::cout << tabs() << "CastExpression : Cast Goal:" << node.getCastGoal().getTypeName() << "\n";
	tabcount_++;
	std::cout << tabs() << "Child:\n";
	tabcount_++;

	if (!node.getChild())
	{
		throw Exceptions::ast_malformation("CastExpression node did not have a child.");
		return;
	}

	node.getChild()->accept(*this);
	tabcount_ -= 2;
}
void Dumper::visit(ASTLiteralExpr & node)
{
	std::cout << tabs() << "Literal: " << FValUtils::dumpFVal(node.getVal()) << '\n';
}

void Dumper::visit(ASTVarDecl & node)
{
	std::cout << tabs() << "VarDeclStmt :" << node.getVarAttr().dump() << std::endl;
	if (node.hasInitExpr())
	{
		tabcount_ += 1;
		std::cout << tabs() << "InitExpr\n";
		tabcount_ += 1;
		node.getInitExpr()->accept(*this);
		tabcount_ -= 2;
	}
}

void Dumper::visit(ASTMemberOfExpr & node)
{
	std::cout << tabs() << "MemberOf Expr:\n";
	tabcount_++;
	std::cout << tabs() << "Base:\n";
	tabcount_++;
	node.getBase()->accept(*this);
	tabcount_--;
	std::cout << tabs() << "Member name:" << node.getMemberNameStr() << "\n";
	tabcount_--;
}

void Dumper::visit(ASTDeclRefExpr & node)
{
	std::cout << tabs() << "DeclRef: name: " << node.getDeclnameStr() << std::endl;
}

void Dumper::visit(ASTFunctionCallExpr & node)
{
	std::cout << tabs() << "Function Call\n";
	tabcount_++;
	std::cout << tabs() << "Function:\n";

	tabcount_++;
	if(node.getDeclRefExpr())
		node.getDeclRefExpr()->accept(*this);
	tabcount_--;

	if (node.getExprList()->size())
	{
		std::cout << tabs() << "Args:\n";
		tabcount_++;

		std::size_t count = 0;
		node.getExprList()->iterate([&](auto arg) {
			std::cout << tabs() << "Arg" << count << '\n';

			tabcount_++;
			arg->accept(*this);
			tabcount_--;

			count++;
		});

		tabcount_--;
	}
	tabcount_--;
}

void Dumper::visit(ASTNullStmt& node)
{
	std::cout << tabs() << "Null Statement\n";
}

void Dumper::visit(ASTFunctionDecl & node)
{
	std::cout << tabs() << "Function Declaration : name:" << node.getName() << " return type:" << node.getReturnType().getTypeName() << "\n";
	tabcount_ += 2;
	std::size_t counter = 0;
	node.iterateArgs([&](auto argdecl){
		std::cout << tabs() << "Arg" << counter << ":" << argdecl.dump() << std::endl;
		counter += 1;
	});
	tabcount_ -= 1;
	std::cout << tabs() << "Body:" << std::endl;
	tabcount_ += 1;
	node.getBody()->accept(*this);
	tabcount_ -= 2;
}

void Dumper::visit(ASTReturnStmt & node)
{
	std::cout << tabs() << "Return statement\n";
	if (node.hasExpr())
	{
		tabcount_ += 1;
		node.getExpr()->accept(*this);
		tabcount_ -= 1;
	}
}

void Dumper::visit(ASTCompoundStmt & node)
{
	std::cout << tabs() << "Compound Statement (Contains " << node.size() << " statements)\n";

	tabcount_ += 1;

	node.iterateStmts([&](auto stmt) {
		stmt->accept(*this);
	});

	tabcount_ -= 1;
}

void Dumper::visit(ASTCondStmt & node)
{
	std::cout << tabs() << "Condition\n";
	tabcount_++;
	// if
	std::cout << tabs() << "Expression (Condition):\n";
	tabcount_++;
	node.getCond()->accept(*this);
	tabcount_--;
	std::cout << tabs() << "Body:\n";
	tabcount_++;
	node.getThen()->accept(*this);
	tabcount_--;
	// has else?
	if (node.getElse())
	{
		std::cout << tabs() << "Else:\n";
		tabcount_++;
		node.getElse()->accept(*this);
		tabcount_--;
	}
	tabcount_--;
}

void Dumper::visit(ASTWhileStmt & node)
{
	std::cout << tabs() << "While Loop\n";

	tabcount_++;
	std::cout << tabs() << "Expression:\n";

	tabcount_++;
	node.getCond()->accept(*this);
	tabcount_--;

	std::cout << tabs() << "Body:\n";
	
	tabcount_++;
	node.getBody()->accept(*this);
	tabcount_--;

	tabcount_--;
}

std::string Dumper::tabs() const
{
	std::string i;
	for (unsigned char k(0); k < tabcount_; k++)
		i += "    ";
	if (tabcount_ > 1)
		i += u8"┗";
	return i;
}


