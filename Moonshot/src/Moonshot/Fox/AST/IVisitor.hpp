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
			inline virtual ~IVisitor() = 0						{}

			inline virtual void visit(ASTBinaryExpr&)		{}
			inline virtual void visit(ASTUnaryExpr&)		{}
			inline virtual void visit(ASTCastExpr&)			{}
			inline virtual void visit(ASTLiteral&)			{}
			inline virtual void visit(ASTVarCall&)			{}

			inline virtual void visit(ASTNullStmt&)			{}
			inline virtual void visit(ASTVarDeclStmt&)		{}

			inline virtual void visit(ASTCompoundStmt&)			{}
			inline virtual void visit(ASTCondition&)		{}
			inline virtual void visit(ASTWhileLoop&)		{}

			inline virtual void visit(ASTFunctionDeclaration&) {}
	};

	template<typename TYPE>
	class ITypedVisitor : public IVisitor
	{
		public:
			inline virtual void visit(ASTBinaryExpr&)	{ value_ = TYPE(); }
			inline virtual void visit(ASTUnaryExpr&)	{ value_ = TYPE(); }
			inline virtual void visit(ASTCastExpr&)		{ value_ = TYPE(); }
			inline virtual void visit(ASTLiteral&)		{ value_ = TYPE(); }
			inline virtual void visit(ASTVarCall&)		{ value_ = TYPE(); }

			inline virtual void visit(ASTNullStmt&)		{ value_ = TYPE(); }
			inline virtual void visit(ASTVarDeclStmt&)	{ value_ = TYPE(); }

			inline virtual void visit(ASTCompoundStmt&)		{ value_ = TYPE(); }
			inline virtual void visit(ASTCondition&)	{ value_ = TYPE(); }
			inline virtual void visit(ASTWhileLoop&)	{ value_ = TYPE(); }

			inline virtual void visit(ASTFunctionDeclaration&) { value_ = TYPE(); }

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


