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
			auto iexpr = node.initExpr_->accept(exprvisitor_);
			symtab_->declareValue(
				node.vattr_,
				iexpr
			);
		}
		else // without
		{
			symtab_->declareValue(
				node.vattr_
			);
		}
	}
	return FVal(); // does not return anything.
}

FVal RTStmtVisitor::visit(ASTExpr & node)
{
	node.accept(exprvisitor_);
	return FVal();
}

void RTStmtVisitor::setSymbolsTable(std::shared_ptr<SymbolsTable> symtab)
{
	symtab_ = symtab;
	exprvisitor_.setSymbolsTable(symtab_);
}

bool RTStmtVisitor::isSymbolsTableAvailable() const
{
	return symtab_ ? true : false;
}
