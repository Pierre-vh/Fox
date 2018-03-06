////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TypeCheck.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// TypeCheck visitor.
// This visitor checks for compatibility between operations, and general type-related semantics.
// e.g. can't multiply a string with a int
//
// This visitor also checks that with variables. It gather informations about them (in datamap_)
// when they are declared, and when a variable is used it checks if it was declared
// and if the type is compatible with the current operation.
////------------------------------------------------------////

#pragma once

#include "Moonshot/Fox/AST/IVisitor.hpp" // base class
#include "Moonshot/Common/Types/Types.hpp"
#include "Moonshot/Fox/Common/Operators.hpp" // enums
#include "Moonshot/Common/DataMap/DataMap.hpp" // DataMap used as a symbols table; It's only temporary, because at this stage I Don't have scopes, so this does the job. Inefficiently, but it does it.

namespace Moonshot
{
	enum class binaryOperator;
	class Context;
	class TypeCheckVisitor : public ITypedVisitor<FoxType>
	{
		public:
			TypeCheckVisitor(Context& c,const bool& testmode = false);
			~TypeCheckVisitor();

			virtual void visit(ASTBinaryExpr & node) override;
			virtual void visit(ASTUnaryExpr & node) override;
			virtual void visit(ASTCastExpr & node) override;

			virtual void visit(ASTLiteralExpr & node) override;

			virtual void visit(ASTVarDecl & node) override;
			virtual void visit(ASTVarRefExpr & node) override;

			DataMap datamap_; // The symbols table used to track variable declarations and types.
			// it is public so we can add anything we want to it for testing purposes.
		private:
			// Directions enum
			enum class directions
			{
				UNKNOWN, LEFT, RIGHT
			};
			// Context
			Context& context_;

			template<typename T>
			FoxType visitAndGetResult(T* node,const directions& dir = directions::UNKNOWN, const binaryOperator& c_binop = binaryOperator::PASS)
			{
				node_ctxt_.cur_binop = c_binop;
				node_ctxt_.dir = dir;

				node->accept(*this);

				node_ctxt_.cur_binop = binaryOperator::PASS;
				node_ctxt_.dir = directions::UNKNOWN;
				return value_;
			}

			bool isAssignable(const IASTExpr* op) const;
			bool shouldOpReturnFloat(const binaryOperator& op) const; // used for operations that return float instead of normal values
			FoxType getExprResultType(const binaryOperator& op, FoxType& lhs, FoxType& rhs);

			struct nodecontext
			{
				directions dir;
				binaryOperator cur_binop;
			} node_ctxt_;
	};

}

