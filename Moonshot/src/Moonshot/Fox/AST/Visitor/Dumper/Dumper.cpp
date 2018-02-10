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
// FVal Utilities
#include "Moonshot/Common/Types/TypesUtils.hpp"
#include "Moonshot/Common/Utils/Utils.hpp" // for enumAsInt
// Include nodes
#include "Moonshot/Fox/AST/Nodes/ASTExpr.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTVarDeclStmt.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTCompStmt.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTCondition.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTWhileLoop.hpp"

#include <iostream>

using namespace Moonshot;
using namespace fv_util;

Dumper::~Dumper()
{
}

void Dumper::visit(ASTBinaryExpr & node)
{
	std::string op = getFromDict(kBinop_dict, node.op_);
	if (op.size() == 0)
		op = util::enumAsInt(node.op_);

	std::cout << tabs() << "BinaryExpression : Operator " << op;
	// print planned result type if there's one
	if (node.resultType_ != 0 && node.resultType_ != indexes::invalid_index)
		std::cout << ", Return type : " << indexToTypeName(node.resultType_);
	// newline
	std::cout << "\n";

	if (!node.right_ || !node.left_)
	{
		throw Exceptions::ast_malformation("BinaryExpression node did not have a left and right child.");
		return;
	}
	else
	{
		// PRINT LEFT CHILD
		tabcount++;
		std::cout << tabs() << "Left child:\n";
		tabcount++;
		node.left_->accept(*this);
		tabcount -= 2;
		// PRINT RIGHT CHILD
		tabcount++;
		std::cout << tabs() << "Right child:\n";
		tabcount++;
		node.right_->accept(*this);
		tabcount -= 2;
	}
	
}

void Dumper::visit(ASTUnaryExpr & node)
{
	std::string op = getFromDict(kUop_dict, node.op_);
	if (op.size() == 0)
		op = util::enumAsInt(node.op_);

	std::cout << tabs() << "UnaryExpression : Operator " << op;

	if (node.resultType_ > 10)
		std::cout << "";

	if (node.resultType_ != 0 && node.resultType_ != indexes::invalid_index)
		std::cout << ", Return type : " << indexToTypeName(node.resultType_);

	std::cout << "\n";

	tabcount++;
	std::cout << tabs() << "Child:\n";
	tabcount++;

	if (!node.child_)
	{
		throw Exceptions::ast_malformation("UnaryExpression node did not have a child.");
		return;
	}

	node.child_->accept(*this);
	tabcount -= 2;
}

void Dumper::visit(ASTCastExpr & node)
{
	std::cout << tabs() << "CastExpression : Cast Goal:" << indexToTypeName(node.getCastGoal()) << "\n";
	tabcount++;
	std::cout << tabs() << "Child:\n";
	tabcount++;

	if (!node.child_)
	{
		throw Exceptions::ast_malformation("CastExpression node did not have a child.");
		return;
	}

	node.child_->accept(*this);
	tabcount -= 2;
}
void Dumper::visit(ASTLiteral & node)
{
	std::cout << tabs() << "Literal: " << dumpFVal(node.val_) << '\n';
}

void Dumper::visit(ASTVarDeclStmt & node)
{
	std::cout << tabs() << "VarDeclStmt :" << dumpVAttr(node.vattr_) << std::endl;
	if (node.initExpr_)
	{
		tabcount += 1;
		std::cout << tabs() << "InitExpr\n";
		tabcount += 1;
		node.initExpr_->accept(*this);
		tabcount -= 2;
	}
}

void Dumper::visit(ASTVarCall & node)
{
	std::cout << tabs() << "VarCall: name: " << node.varname_ << std::endl;
}

void Dumper::visit(ASTNullStmt& node)
{
	std::cout << tabs() << "Null Statement\n";
}

void Dumper::visit(ASTCompStmt & node)
{
	std::cout << tabs() << "Compound Statement (Contains " << node.statements_.size() << " statements)\n";

	tabcount += 1;

	for (auto& elem : node.statements_)
		elem->accept(*this);

	tabcount -= 1;
}

void Dumper::visit(ASTCondition & node)
{
	std::cout << tabs() << "Condition Branch\n";
	int counter = 0;
	// (else) ifs
	for (auto& elem : node.conditional_stmts_)
	{
		tabcount++;
		std::cout << tabs() << "Condition " << counter << std::endl;
		tabcount++;

		std::cout << tabs() << "Condition Expression:\n";
		tabcount++;
		elem.expr_->accept(*this);
		tabcount--;

		std::cout << tabs() << "Condition Body:\n";

		tabcount++;
		elem.stmt_->accept(*this);
		tabcount-=3;

		counter++;
	}
	// has else?
	if (node.else_stmt_)
	{
		tabcount++;
		std::cout << tabs() << "\"Else\" Body:\n";
		tabcount++;
		node.else_stmt_->accept(*this);
		tabcount -= 2;
	}
}

void Dumper::visit(ASTWhileLoop & node)
{
	std::cout << tabs() << "While Loop\n";

	tabcount++;
	std::cout << tabs() << "Expression:\n";

	tabcount++;
	node.expr_->accept(*this);
	tabcount--;

	std::cout << tabs() << "Body:\n";
	
	tabcount++;
	node.body_->accept(*this);
	tabcount--;

	tabcount--;
}

std::string Dumper::tabs() const
{
	std::string i;
	for (unsigned char k(0); k < tabcount; k++)
		i += '\t';
	if (tabcount > base_tabs_)
		i += '\xC0';
	return i;
}


