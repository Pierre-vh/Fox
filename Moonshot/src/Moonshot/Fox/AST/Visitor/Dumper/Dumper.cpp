////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Dumper.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Dumper.h"

using namespace Moonshot;
using namespace fv_util;

Dumper::Dumper()
{
	//std::cout << "Visitor \"Dumper\" Initialized. Dumping tree:" << std::endl;
}

Dumper::~Dumper()
{
}

void Dumper::visit(ASTExpr & node)
{
	std::cout << tabs() << "Expression : Operator ";
	// Attempts to print the operator in a str form
	auto strOp = kOptype_dict.find(node.op_);
	if (strOp != kOptype_dict.end())
		std::cout << strOp->second;
	else
		std::cout << util::enumAsInt(node.op_);

	if (node.totype_ != invalid_index)
		std::cout << ", Return type : " << indexToTypeName(node.totype_);
	std::cout << std::endl;
	if (node.left_)
	{
		tabcount++;
		std::cout << tabs() << "Left child:" << std::endl;
		tabcount++;
		node.left_->accept(*this);
		tabcount -= 2;
	}
	if (node.right_)
	{
		tabcount++;
		std::cout << tabs() << "Right child:" << std::endl;
		tabcount++;
		node.right_->accept(*this);
		tabcount -= 2;
	}
}

void Dumper::visit(ASTLiteral & node)
{
	std::cout << tabs() << "Literal: " << dumpFVal(node.val_) << std::endl;
}

void Dumper::visit(ASTVarDeclStmt & node)
{
	std::cout << tabs() << "VarDeclStmt :" << dumpVAttr(node.vattr_) << std::endl;
	if (node.initExpr_)
	{
		tabcount += 1;
		std::cout << tabs() << "InitExpr" << std::endl;
		tabcount += 1;
		node.initExpr_->accept(*this);
		tabcount -= 2;
	}
}

void Dumper::visit(ASTVarCall & node)
{
	std::cout << tabs() << "VarCall: name: " << node.varname_ << std::endl;
}

void Dumper::visit(ASTCompStmt & node)
{
	std::cout << tabs() << "Compound Statement (Contains " << node.statements_.size() << " statements)" << std::endl;

	tabcount += 1;

	for (auto& elem : node.statements_)
		elem->accept(*this);

	tabcount -= 1;
}

void Dumper::visit(ASTCondition & node)
{
	std::cout << tabs() << "Condition Branch" << std::endl;
	int counter = 0;
	// (else) ifs
	for (auto& elem : node.conditional_blocks_)
	{
		tabcount++;
		std::cout << tabs() << "Condition " << counter << std::endl;
		tabcount++;

		std::cout << tabs() << "Condition Expression:" << std::endl;
		tabcount++;
		elem.first->accept(*this);
		tabcount--;

		std::cout << tabs() << "Condition Body:" << std::endl;

		tabcount++;
		elem.second->accept(*this);
		tabcount-=3;

		counter++;
	}
	// has else?
	if (node.else_block_)
	{
		tabcount++;
		std::cout << tabs() << "\"Else\" Body:" << std::endl;
		tabcount++;
		node.else_block_->accept(*this);
		tabcount -= 2;
	}
}

std::string Dumper::tabs() const
{
	std::string i;
	for (unsigned int k(0); k < tabcount; k++)
		i += '\t';
	if (tabcount > 1)
		i += '\xC0';
	return i;
}


