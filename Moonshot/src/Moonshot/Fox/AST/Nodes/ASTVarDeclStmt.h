////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTVarDeclStmt.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// The AST Node for Variable Declaration Statements.											
////------------------------------------------------------////

#pragma once

#include "IASTStmt.h"
#include "ASTExpr.h"

#include "../../../Common/Types/Types.h"
#include "../Visitor/Dumper/Dumper.h"

#include <string> // std::string
#include <memory> // std::unique_ptr, std::make_unique

namespace Moonshot 
{
	struct ASTVarDeclStmt : public IASTStmt
	{
		public:
			// Create a variable declaration statement by giving the constructor the variable's properties (name,is const and type) and, if there's one, an expression to initialize it.
			ASTVarDeclStmt(const var::varattr &attr,std::unique_ptr<ASTExpr>& iExpr); 
			~ASTVarDeclStmt();

			// Inherited via IASTStmt
			virtual void accept(IVisitor& vis) override;

			var::varattr vattr_;
			std::unique_ptr<ASTExpr> initExpr_;
	};
}


