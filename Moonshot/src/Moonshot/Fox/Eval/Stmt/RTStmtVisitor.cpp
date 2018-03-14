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
#include "Moonshot/Fox/AST/Nodes/ASTExpr.hpp"
#include "Moonshot/Common/Utils/Utils.hpp"
#include "Moonshot/Common/Context/Context.hpp" // Context 
#include "Moonshot/Common/DataMap/DataMap.hpp" // Symbols table
#include "Moonshot/Fox/AST/Nodes/ASTDecl.hpp" // Nodes

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

void RTStmtVisitor::visit(ASTVarDecl & node)
{
	if (!isDataMapAvailable())
		context_.logMessage("Can't Visit VarDeclStmt nodes when the symbols table is not available.");
	else
	{
		if (node.hasInitExpr()) // With init expr
		{
			node.getInitExpr()->accept(*this);
			auto iexpr = value_;
			if (!symtab_declareValue_derefFirst(
				node.getVarAttr(),
				iexpr
			))
				context_.reportError("Error while initializing variable " + node.getVarAttr().name_);
		}
		else // without
		{
			if(!symtab_declareValue_derefFirst(
				node.getVarAttr()
			))
			context_.reportError("Error while initializing variable " + node.getVarAttr().name_);
		}
	}
	value_ = FoxValue(); // does not return anything.
}

bool RTStmtVisitor::symtab_declareValue_derefFirst(const FoxVariableAttr & vattr, FoxValue initval)
{
	if (std::holds_alternative<FoxVariableRef>(initval))
		initval = datamap_->retrieveValue(std::get<FoxVariableRef>(initval).getName());
	return datamap_->declareValue(vattr, initval);;
}
