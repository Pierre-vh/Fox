////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IVisitor.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Declaration of the abstract class IVisitor, used as a base for 
// every compile time visitor, and visitors in general (except runtime ones)
// see IRTVisitor for that.
//
// The class is abstract, but implements the visit method for each node.
// So, when you implement a visitor, you override only the functions you need,
// and the one you didn't implemented will default to the ones here, instead
// of throwing an error.
////------------------------------------------------------////

#pragma once
//nodes
#include "Moonshot/Fox/AST/Decl.hpp"
#include "Moonshot/Fox/AST/Expr.hpp"
#include "Moonshot/Fox/AST/Stmt.hpp"

namespace Moonshot
{
	class IVisitor
	{
		public:
			inline virtual ~IVisitor() = 0						{}

			inline virtual void visit(UnitDecl&)				{}
			
			inline virtual void visit(BinaryExpr&)			{}
			inline virtual void visit(UnaryExpr&)			{}
			inline virtual void visit(CastExpr&)				{}
			inline virtual void visit(CharLiteralExpr&)		{}
			inline virtual void visit(IntegerLiteralExpr&)	{}
			inline virtual void visit(FloatLiteralExpr&)		{}
			inline virtual void visit(StringLiteralExpr&)	{}
			inline virtual void visit(BoolLiteralExpr&)		{}
			inline virtual void visit(ArrayLiteralExpr&) {}
			inline virtual void visit(DeclRefExpr&)		{}
			inline virtual void visit(ArrayAccessExpr&)		{}
			inline virtual void visit(FunctionCallExpr&) {}

			inline virtual void visit(NullExpr&)		{}
			inline virtual void visit(VarDecl&)		{}

			inline virtual void visit(CompoundStmt&)	{}
			inline virtual void visit(ConditionStmt&)		{}
			inline virtual void visit(WhileStmt&)	{}

			inline virtual void visit(ArgDecl&)		{}
			inline virtual void visit(FunctionDecl&) {}
			inline virtual void visit(ReturnStmt&)	{}
	};
}


