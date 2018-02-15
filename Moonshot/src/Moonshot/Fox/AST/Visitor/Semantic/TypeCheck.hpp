////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TypeCheck.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// TypeCheckVisitor visitor.
// This visitor checks for compatibility between operations :
// e.g. can't multiply a string with a int
//
// This visitor also checks that with variables. It gather informations about them (in datamap_)
// when they are declared, and when a variable is used it checks if it was declared
// and if the type is compatible with the current operation.
////------------------------------------------------------////

#pragma once

#include "Moonshot/Fox/AST/Visitor/IVisitor.hpp" // base class
#include "Moonshot/Fox/Util/Enums.hpp" // enums
#include "Moonshot/Common/DataMap/DataMap.hpp" // DataMap used as a symbols table; It's only temporary, because at this stage I Don't have scopes, so this does the job. Inefficiently, but it does it.

namespace Moonshot
{
	enum class binaryOperation;
	class Context;
	class TypeCheckVisitor : public ITypedVisitor<std::size_t> // size_t because we return indexes in FVal to represent types.
	{
		public:
			TypeCheckVisitor(Context& c,const bool& testmode = false);
			~TypeCheckVisitor();

			virtual void visit(ASTBinaryExpr & node) override;
			virtual void visit(ASTUnaryExpr & node) override;
			virtual void visit(ASTCastExpr & node) override;

			virtual void visit(ASTLiteral & node) override;

			virtual void visit(ASTVarDeclStmt & node) override;
			virtual void visit(ASTVarCall & node) override;

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
			std::size_t visitAndGetResult(T* node,const directions& dir = directions::UNKNOWN, const binaryOperation& c_binop = binaryOperation::PASS)
			{
				node_ctxt_.cur_binop = c_binop;
				node_ctxt_.dir = dir;

				node->accept(*this);

				node_ctxt_.cur_binop = binaryOperation::PASS;
				node_ctxt_.dir = directions::UNKNOWN;
				return value_;
			}

			bool isAssignable(const IASTExpr* op) const;
			bool shouldOpReturnFloat(const binaryOperation& op) const; // used for operations that return float instead of normal values
			std::size_t getExprResultType(const binaryOperation& op, std::size_t& lhs, const std::size_t& rhs);

			struct nodecontext
			{
				directions dir;
				binaryOperation cur_binop;
			} node_ctxt_;
	};

}

