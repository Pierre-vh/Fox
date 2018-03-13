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
//utils
#include "Moonshot/Common/Utils/Utils.hpp"
#include "Moonshot/Common/Types/Types.hpp"

#include "Moonshot/Fox/AST/Nodes/ForwardDeclarations.hpp"

namespace Moonshot
{
	class IVisitor
	{
		public:
			inline virtual ~IVisitor() = 0				{}

			inline virtual void visit(ASTBinaryExpr&)	{}
			inline virtual void visit(ASTUnaryExpr&)	{}
			inline virtual void visit(ASTCastExpr&)		{}
			inline virtual void visit(ASTLiteralExpr&)	{}
			inline virtual void visit(ASTDeclRefExpr&)	{}
			inline virtual void visit(ASTMemberRefExpr&)	{}
			inline virtual void visit(ASTFunctionCallExpr&) {}

			inline virtual void visit(ASTNullStmt&)		{}
			inline virtual void visit(ASTVarDecl&)		{}

			inline virtual void visit(ASTCompoundStmt&)	{}
			inline virtual void visit(ASTCondStmt&)		{}
			inline virtual void visit(ASTWhileStmt&)	{}

			inline virtual void visit(ASTFunctionDecl&) {}
			inline virtual void visit(ASTReturnStmt&)	{}
	};

	template<typename TYPE>
	class ITypedVisitor : public IVisitor
	{
		public:
			inline virtual void visit(ASTBinaryExpr&)	{ value_ = TYPE(); }
			inline virtual void visit(ASTUnaryExpr&)	{ value_ = TYPE(); }
			inline virtual void visit(ASTCastExpr&)		{ value_ = TYPE(); }
			inline virtual void visit(ASTLiteralExpr&)	{ value_ = TYPE(); }
			inline virtual void visit(ASTDeclRefExpr&)	{ value_ = TYPE(); }
			inline virtual void visit(ASTMemberRefExpr&) { value_ = TYPE(); }
			inline virtual void visit(ASTFunctionCallExpr&) { value_ = TYPE(); }

			inline virtual void visit(ASTNullStmt&)		{ value_ = TYPE(); }
			inline virtual void visit(ASTVarDecl&)		{ value_ = TYPE(); }

			inline virtual void visit(ASTCompoundStmt&)	{ value_ = TYPE(); }
			inline virtual void visit(ASTCondStmt&)		{ value_ = TYPE(); }
			inline virtual void visit(ASTWhileStmt&)	{ value_ = TYPE(); }

			inline virtual void visit(ASTFunctionDecl&) { value_ = TYPE(); }
			inline virtual void visit(ASTReturnStmt&)	{ value_ = TYPE(); }

			virtual ~ITypedVisitor() = 0	{}
		protected:
			TYPE value_ = TYPE();

			template<typename NODE,typename VISITOR>
			TYPE visitAndGetResult(NODE* node,VISITOR& visit)
			{
				node->accept(visit);
				return value_;
			}
	};

}


