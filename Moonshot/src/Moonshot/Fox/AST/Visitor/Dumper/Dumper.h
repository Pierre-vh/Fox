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
#include "../../../../Common/Types/TypesUtils.h"
#include "../../../../Common/Utils/Utils.h" // for enumAsInt
// Include nodes
#include "../../Nodes/ASTExpr.h"
#include "../../Nodes//ASTVarDeclStmt.h"
#include "../../Nodes/ASTCompStmt.h"
#include "../../Nodes/ASTCondition.h"
#include "../../Nodes/ASTWhileLoop.h"

namespace Moonshot
{
	class Dumper : public IVisitor
	{
		public:
			Dumper() = default;
			~Dumper();

			virtual void visit(ASTBinaryExpr & node) override;
			virtual void visit(ASTUnaryExpr & node) override;
			virtual void visit(ASTCastExpr & node) override;
			virtual void visit(ASTLiteral & node) override;
			virtual void visit(ASTVarDeclStmt & node) override;
			virtual void visit(ASTVarCall & node) override;
			virtual void visit(ASTCompStmt & node) override;
			virtual void visit(ASTCondition & node) override;
			virtual void visit(ASTWhileLoop & node) override;
		private:
			static constexpr unsigned char base_tabs_ = 1; // set this number to a higher value to have a offset when dumping ast

			std::string tabs() const;
			unsigned char tabcount = base_tabs_; 
	};
}


