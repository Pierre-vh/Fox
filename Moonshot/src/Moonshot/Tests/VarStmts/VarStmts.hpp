////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : VarStmts.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Var stmt tests : declarations and assignements/use of variables !										
////------------------------------------------------------////

#pragma once

#include "../ITest.hpp"

#include "Moonshot/Fox/AST/Visitor/Semantic/TypeCheck.hpp"
#include "Moonshot/Fox/AST/Visitor/Eval/Stmt/RTStmtVisitor.hpp"

namespace Moonshot::Test
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


