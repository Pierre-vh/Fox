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
