////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : RTStmtVisitor.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
// Implementation of the Runtime Statement Visitor.
////------------------------------------------------------////

#include "RTStmtVisitor.hpp"

#include "Moonshot/Common/Utils/Utils.hpp"
#include "Moonshot/Common/Context/Context.hpp" // Context 
#include "Moonshot/Common/DataMap/DataMap.hpp" // Symbols table
#include "Moonshot/Fox/AST/Nodes/ASTVarDeclStmt.hpp" // Nodes

using namespace Moonshot;

RTStmtVisitor::RTStmtVisitor(Context& c) : RTExprVisitor(c) // call superclass constructor
{
	
}

RTStmtVisitor::RTStmtVisitor(Context& c,std::shared_ptr<DataMap> symtab) : RTExprVisitor(c) // call superclass constructor
{
	setDataMap(symtab);
}

RTStmtVisitor::~RTStmtVisitor()
{
}

void RTStmtVisitor::visit(ASTVarDeclStmt & node)
{
	if (!isDataMapAvailable())
		context_.logMessage("Can't Visit VarDeclStmt nodes when the symbols table is not available.");
	else
	{
		if (node.initExpr_) // With init expr
		{
			node.initExpr_->accept(*this);
			auto iexpr = value_;
			if (!symtab_declareValue_derefFirst(
				node.vattr_,
				iexpr
			))
				context_.reportError("Error while initializing variable " + node.vattr_.name_);
		}
		else // without
		{
			if(!symtab_declareValue_derefFirst(
				node.vattr_
			))
			context_.reportError("Error while initializing variable " + node.vattr_.name_);
		}
	}
	value_ = FVal(); // does not return anything.
}

bool RTStmtVisitor::symtab_declareValue_derefFirst(const var::varattr & vattr, FVal initval)
{
	if (std::holds_alternative<var::varRef>(initval))
		initval = datamap_->retrieveValue(std::get<var::varRef>(initval).getName());
	return datamap_->declareValue(vattr, initval);;
}
