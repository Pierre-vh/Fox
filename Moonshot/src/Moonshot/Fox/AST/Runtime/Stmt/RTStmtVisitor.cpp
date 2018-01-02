////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : RTStmtVisitor.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
// Implementation of the Runtime Statement Visitor.
////------------------------------------------------------////

#include "RTStmtVisitor.h"

using namespace Moonshot;

RTStmtVisitor::RTStmtVisitor()
{
}

RTStmtVisitor::RTStmtVisitor(std::shared_ptr<SymbolsTable> symtab)
{
	setSymbolsTable(symtab);
}

RTStmtVisitor::~RTStmtVisitor()
{
}

FVal RTStmtVisitor::visit(ASTVarDeclStmt & node)
{
	if (!isSymbolsTableAvailable())
		E_LOG("Can't Visit VarDeclStmt nodes when the symbols table is not available.");
	else
	{
		if (node.initExpr_) // With init expr
		{
			auto iexpr = node.initExpr_->accept(*this);
			if (!symtab_declareValue_derefFirst(
				node.vattr_,
				iexpr
			))
				E_ERROR("Error while initializing variable " + node.vattr_.name);
		}
		else // without
		{
			if(!symtab_declareValue_derefFirst(
				node.vattr_
			))
			E_ERROR("Error while initializing variable " + node.vattr_.name);
		}
	}
	return FVal(); // does not return anything.
}

bool RTStmtVisitor::symtab_declareValue_derefFirst(const var::varattr & vattr, FVal initval)
{
	if (std::holds_alternative<var::varRef>(initval))
		initval = symtab_->retrieveValue(std::get<var::varRef>(initval).getName());
	return symtab_->declareValue(vattr, initval);;
}
