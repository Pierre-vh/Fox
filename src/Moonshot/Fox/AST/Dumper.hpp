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
#include <iostream>
#include "ASTUnit.hpp"
#include "Moonshot/Fox/AST/IVisitor.hpp"

namespace Moonshot
{
	class Dumper : public IVisitor
	{
		public:
			Dumper(std::ostream& outstream = std::cout,const unsigned char& offsettabs = 0);

			void dumpUnit(ASTUnit & unit);

			virtual void visit(ASTBinaryExpr & node) override;
			virtual void visit(ASTUnaryExpr & node) override;
			virtual void visit(ASTCastExpr & node) override;
			virtual void visit(ASTLiteralExpr & node) override;
			virtual void visit(ASTVarDecl & node) override;
			virtual void visit(ASTMemberAccessExpr & node) override;
			virtual void visit(ASTArrayAccess & node) override;
			virtual void visit(ASTDeclRefExpr & node) override;
			virtual void visit(ASTFunctionCallExpr & node) override;
			virtual void visit(ASTCompoundStmt & node) override;
			virtual void visit(ASTCondStmt & node) override;
			virtual void visit(ASTWhileStmt & node) override;
			virtual void visit(ASTNullStmt&) override;
			virtual void visit(ASTFunctionDecl& node) override;
			virtual void visit(ASTReturnStmt& node) override;
		private:
			std::ostream &out_;
			std::string getIndent() const;
			std::string getOffsetTabs() const;
			unsigned char curindent_ = 1,offsetTabs_ = 0; 
	};
}


