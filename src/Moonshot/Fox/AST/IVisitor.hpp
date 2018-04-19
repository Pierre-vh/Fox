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
#include "Moonshot/Fox/AST/ASTDecl.hpp"
#include "Moonshot/Fox/AST/ASTExpr.hpp"
#include "Moonshot/Fox/AST/ASTStmt.hpp"

namespace Moonshot
{
	class IVisitor
	{
		public:
			inline virtual ~IVisitor() = 0						{}

			inline virtual void visit(ASTBinaryExpr&)			{}
			inline virtual void visit(ASTUnaryExpr&)			{}
			inline virtual void visit(ASTCastExpr&)				{}
			inline virtual void visit(ASTCharLiteralExpr&)		{}
			inline virtual void visit(ASTIntegerLiteralExpr&)	{}
			inline virtual void visit(ASTFloatLiteralExpr&)		{}
			inline virtual void visit(ASTStringLiteralExpr&)	{}
			inline virtual void visit(ASTBoolLiteralExpr&)		{}
			inline virtual void visit(ASTArrayLiteralExpr&) {}
			inline virtual void visit(ASTDeclRefExpr&)		{}
			inline virtual void visit(ASTMemberAccessExpr&)	{}
			inline virtual void visit(ASTArrayAccess&)		{}
			inline virtual void visit(ASTFunctionCallExpr&) {}

			inline virtual void visit(ASTNullExpr&)		{}
			inline virtual void visit(ASTVarDecl&)		{}

			inline virtual void visit(ASTCompoundStmt&)	{}
			inline virtual void visit(ASTCondStmt&)		{}
			inline virtual void visit(ASTWhileStmt&)	{}

			inline virtual void visit(ASTArgDecl&)		{}
			inline virtual void visit(ASTFunctionDecl&) {}
			inline virtual void visit(ASTReturnStmt&)	{}
	};
}


