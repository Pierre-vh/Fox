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
#include "../../../Common/Utils/Utils.hpp"
#include "../../../Common/Types/Types.hpp"

namespace Moonshot
{
	struct ASTLiteral; 
	struct ASTVarDeclStmt; 
	struct ASTVarCall; 
	struct ASTCompStmt; 
	struct ASTCondition;
	struct ASTWhileLoop;
	struct IASTExpr;
	struct ASTBinaryExpr;
	struct ASTUnaryExpr;
	struct ASTCastExpr;
	class IVisitor
	{
		public:
			inline virtual ~IVisitor() = 0						{}

			inline virtual void visit(ASTBinaryExpr &node)		{}
			inline virtual void visit(ASTUnaryExpr &node)		{}
			inline virtual void visit(ASTCastExpr &node)		{}
			inline virtual void visit(ASTLiteral &node)			{}
			inline virtual void visit(ASTVarCall& node) {}

			inline virtual void visit(ASTVarDeclStmt &node)		{}

			inline virtual void visit(ASTCompStmt& node)		{}
			inline virtual void visit(ASTCondition& node)		{}
			inline virtual void visit(ASTWhileLoop& node)		{}
	};

	template<typename TYPE>
	class ITypedVisitor : public IVisitor
	{
		public:
			virtual ~ITypedVisitor() = 0
			{

			}
		protected:
			TYPE value_;

			template<typename NODE,typename VISITOR>
			TYPE visitAndGetResult(std::unique_ptr<NODE>& node,VISITOR& visit)
			{
				node->accept(visit);
				return value_;
			}
	};

}


