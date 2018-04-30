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
#include "Moonshot/Fox/AST/IVisitor.hpp"

namespace Moonshot
{
	class Dumper : public IVisitor
	{
		public:
			Dumper(std::ostream& outstream = std::cout,const unsigned char& offsettabs = 0);

			virtual void visit(UnitDecl & node) override;

			virtual void visit(BinaryExpr & node) override;
			virtual void visit(UnaryExpr & node) override;
			virtual void visit(CastExpr & node) override;
			virtual void visit(CharLiteralExpr & node) override;
			virtual void visit(IntegerLiteralExpr & node) override;
			virtual void visit(FloatLiteralExpr & node) override;
			virtual void visit(StringLiteralExpr & node) override;
			virtual void visit(BoolLiteralExpr & node) override;
			virtual void visit(ArrayLiteralExpr& node) override;
			virtual void visit(VarDecl & node) override;
			virtual void visit(ArrayAccessExpr & node) override;
			virtual void visit(MemberOfExpr& node) override;
			virtual void visit(DeclRefExpr & node) override;
			virtual void visit(FunctionCallExpr & node) override;
			virtual void visit(CompoundStmt & node) override;
			virtual void visit(ConditionStmt & node) override;
			virtual void visit(WhileStmt & node) override;
			virtual void visit(DeclStmt & node)	override;
			virtual void visit(NullExpr&) override;
			virtual void visit(ArgDecl& node) override;
			virtual void visit(FunctionDecl& node) override;
			virtual void visit(ReturnStmt& node) override;
		private:
			std::ostream &out_;
			std::string getIndent() const;
			std::string getOffsetTabs() const;
			unsigned char curindent_ = 1,offsetTabs_ = 0; 
	};
}


