////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : RTStmtVisitor.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Runtime visitor for statements.
// Used for debugging purposes. When the VM will be done, this file
// will probably be useless.
////------------------------------------------------------////
#pragma once
// Types & utils
#include "../../../../../Common/Types/Types.h"
#include "../Expr/RTExprVisitor.h" //Superclass

namespace Moonshot
{
	class Context;
	class SymbolsTable;
	// Visits statements nodes : vardecl & exprstmt
	class RTStmtVisitor : public RTExprVisitor // Inherits from the expression visitor, because of expression statements!
	{
		public:
			RTStmtVisitor(Context& c);
			RTStmtVisitor(Context& c,std::shared_ptr<SymbolsTable> symtab);
			~RTStmtVisitor();

			virtual void visit(ASTVarDeclStmt & node) override;

		private:
			// Declares the value, but deref initival if it's a reference.
			// This happens when you have statements such as let foo : int = bar;
			bool symtab_declareValue_derefFirst(const var::varattr& vattr, FVal initval = FVal());
	};

}

