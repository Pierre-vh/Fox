////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Dumper.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// The dumper visitor is a visitors that "dumps" the AST into a text representation.
// Very useful for debugging, or just seeing the AST in general !
////------------------------------------------------------////

#pragma once
// base class
#include "../IVisitor.h"
// FVal Utilities
#include "../../../../Common/Types/Types.h"
#include "../../../../Common/Utils/Utils.h" // for enumAsInt
// Include nodes
#include "../../Nodes/ASTExpr.h"
#include "../../Nodes//ASTVarDeclStmt.h"
#include "../../Nodes/ASTCompStmt.h"
#include "../../Nodes/ASTCondition.h"

#include <Windows.h>

namespace Moonshot
{
	class Dumper : public IVisitor
	{
		public:
			Dumper();
			~Dumper();

			virtual void visit(ASTExpr & node) override;
			virtual void visit(ASTLiteral & node) override;
			virtual void visit(ASTVarDeclStmt & node) override;
			virtual void visit(ASTVarCall & node) override;
			virtual void visit(ASTCompStmt & node) override;
			virtual void visit(ASTCondition & node) override;
		private:
			std::string tabs() const;
			unsigned int tabcount = 1;
	};
}


