////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Dumper.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// The dumper visitor is a visitors that "dumps" the AST into a text representation.
// Very useful for debugging, or just seeing the AST in general !
////------------------------------------------------------////

#pragma once
// base class
#include "Moonshot/Fox/AST/IVisitor.hpp"

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
			virtual void visit(ASTNullStmt& node) override;
			virtual void visit(ASTFunctionDeclaration& node) override;
		private:

			std::string tabs() const;
			unsigned char tabcount_ = 1; 
	};
}


