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
			virtual void visit(ASTLiteralExpr & node) override;
			virtual void visit(ASTVarDecl & node) override;
			virtual void visit(ASTVarRefExpr & node) override;
			virtual void visit(ASTCompoundStmt & node) override;
			virtual void visit(ASTCondStmt & node) override;
			virtual void visit(ASTWhileStmt & node) override;
			virtual void visit(ASTNullStmt& node) override;
			virtual void visit(ASTFunctionDecl& node) override;
			virtual void visit(ASTReturnStmt& node) override;
		private:

			std::string tabs() const;
			unsigned char tabcount_ = 1; 
	};
}


