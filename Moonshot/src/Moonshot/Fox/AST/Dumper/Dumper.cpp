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
#include "Moonshot/Common/Types/FoxValueUtils.hpp"
#include "Moonshot/Common/Types/TypesUtils.hpp"
#include "Moonshot/Common/Utils/Utils.hpp" // for enumAsInt
// Include nodes
#include "Moonshot/Fox/AST/Nodes/ASTExpr.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTVarDeclStmt.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTCompStmt.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTCondition.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTWhileLoop.hpp"
#include "Moonshot/Common/Utils/Utils.hpp"
#include <iostream>

using namespace Moonshot;
using namespace TypeUtils;

Dumper::~Dumper()
{
}

void Dumper::visit(ASTBinaryExpr & node)
{
	std::string op = Util::getFromDict(Dicts::kBinopToStr_dict, node.op_);
	if (op.size() == 0)
		op = Util::enumAsInt(node.op_);

	std::cout << tabs() << "BinaryExpression : Operator " << op;
	// print planned result type if there's one
	if (node.resultType_ != 0 && node.resultType_ != TypeIndex::InvalidIndex)
		std::cout << ", Return type : " << node.resultType_.getTypeName();
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
		tabcount_++;
		std::cout << tabs() << "Left child:\n";
		tabcount_++;
		node.left_->accept(*this);
		tabcount_ -= 2;
		// PRINT RIGHT CHILD
		tabcount_++;
		std::cout << tabs() << "Right child:\n";
		tabcount_++;
		node.right_->accept(*this);
		tabcount_ -= 2;
	}
	
}

void Dumper::visit(ASTUnaryExpr & node)
{
	std::string op = Util::getFromDict(Dicts::kUnaryOpToStr_dict, node.op_);
	if (op.size() == 0)
		op = Util::enumAsInt(node.op_);

	std::cout << tabs() << "UnaryExpression : Operator " << op;

	if (node.resultType_ != 0 && node.resultType_ != TypeIndex::InvalidIndex)
		std::cout << ", Return type : " << node.resultType_.getTypeName();

	std::cout << "\n";

	tabcount_++;
	std::cout << tabs() << "Child:\n";
	tabcount_++;

	if (!node.child_)
	{
		throw Exceptions::ast_malformation("UnaryExpression node did not have a child.");
		return;
	}

	node.child_->accept(*this);
	tabcount_ -= 2;
}

void Dumper::visit(ASTCastExpr & node)
{
	std::cout << tabs() << "CastExpression : Cast Goal:" << node.getCastGoal().getTypeName() << "\n";
	tabcount_++;
	std::cout << tabs() << "Child:\n";
	tabcount_++;

	if (!node.child_)
	{
		throw Exceptions::ast_malformation("CastExpression node did not have a child.");
		return;
	}

	node.child_->accept(*this);
	tabcount_ -= 2;
}
void Dumper::visit(ASTLiteral & node)
{
	std::cout << tabs() << "Literal: " << FValUtils::dumpFVal(node.val_) << '\n';
}

void Dumper::visit(ASTVarDeclStmt & node)
{
	std::cout << tabs() << "VarDeclStmt :" << node.vattr_.dump() << std::endl;
	if (node.initExpr_)
	{
		tabcount_ += 1;
		std::cout << tabs() << "InitExpr\n";
		tabcount_ += 1;
		node.initExpr_->accept(*this);
		tabcount_ -= 2;
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

	tabcount_ += 1;

	for (const auto& elem : node.statements_)
		elem->accept(*this);

	tabcount_ -= 1;
}

void Dumper::visit(ASTCondition & node)
{
	std::cout << tabs() << "Condition\n";
	tabcount_++;
	// if
	std::cout << tabs() << "Expression (Condition):\n";
	tabcount_++;
	node.condition_expr_->accept(*this);
	tabcount_--;
	std::cout << tabs() << "Body:\n";
	tabcount_++;
	node.condition_stmt_->accept(*this);
	tabcount_--;
	// has else?
	if (node.else_stmt_)
	{
		std::cout << tabs() << "Else:\n";
		tabcount_++;
		node.else_stmt_->accept(*this);
		tabcount_--;
	}
	tabcount_--;
}

void Dumper::visit(ASTWhileLoop & node)
{
	std::cout << tabs() << "While Loop\n";

	tabcount_++;
	std::cout << tabs() << "Expression:\n";

	tabcount_++;
	node.expr_->accept(*this);
	tabcount_--;

	std::cout << tabs() << "Body:\n";
	
	tabcount_++;
	node.body_->accept(*this);
	tabcount_--;

	tabcount_--;
}

std::string Dumper::tabs() const
{
	std::string i;
	for (unsigned char k(0); k < tabcount_; k++)
		i += '\t';
	if (tabcount_ > 1)
		i += u8"┗";
	return i;
}


