#pragma once
#include "../../../../Common/Utils/Utils.h"
#include "../../../../Common/Errors/Errors.h"
#include "../../../../Common/Types/Types.h"
#include "../../../../Common/Symbols/Symbols.h"

#include "../../Nodes/ASTExpr.h"
#include "../../Nodes/ASTVarDeclStmt.h"
#include "../Expr/RTExprVisitor.h"

#include "../IRTVisitor.h"

namespace Moonshot
{
	// Visits statements nodes : vardecl & exprstmt
	class RTStmtVisitor : public IRTVisitor
	{
		public:
			RTStmtVisitor();
			RTStmtVisitor(std::shared_ptr<SymbolsTable> symtab);
			~RTStmtVisitor();

			virtual FVal visit(ASTVarDeclStmt & node) override;
			virtual FVal visit(ASTExpr & node) override;

			void setSymbolsTable(std::shared_ptr<SymbolsTable> symtab);
		private:
			RTExprVisitor exprvisitor_;
			bool isSymbolsTableAvailable() const;
			std::shared_ptr<SymbolsTable> symtab_;
	};

}

