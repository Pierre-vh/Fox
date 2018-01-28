////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : VarStmts.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Var stmt tests : declarations and assignements/use of variables !										
////------------------------------------------------------////

#pragma once

#include "../ITest.h"

#include "../../Fox/AST/CompTime/Semantic/TypeCheck.h"
#include "../../Fox/AST/Visitor/Eval/Stmt/RTStmtVisitor.h"

namespace Moonshot
{
	class VarStmts : public ITest
	{
		public:
			VarStmts();
			~VarStmts();

			virtual std::string getTestName() const override;
			virtual bool runTest(Context & context) override;
	};
}


