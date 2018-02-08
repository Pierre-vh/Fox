////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TypeCheck.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// TypeCheckVisitor visitor.
// This visitor checks for compatibility between operations :
// e.g. can't multiply a string with a int
//
// This visitor also checks that with variables. It gather informations about them (in symtable_)
// when they are declared, and when a variable is used it checks if it was declared
// and if the type is compatible with the current operation.
////------------------------------------------------------////

#pragma once

#include "../../Visitor/IVisitor.h" // base class
#include "../../../Util/Enums.h" // enums
#include "../../../../Common/Symbols/Symbols.h" // symbols table

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

			SymbolsTable symtable_; // The symbols table used to track variable declarations and types.
			// it is public so we can add anything we want to it for testing purposes.
		private:
			// Context
			Context& context_;

			template<typename T>
			inline std::size_t visitAndGetResult(std::unique_ptr<T>& node,const dir& dir = dir::UNKNOWNDIR, const binaryOperation& c_binop = binaryOperation::PASS)
			{
				node_ctxt_.cur_binop = c_binop;
				node_ctxt_.dir = dir;

				node->accept(*this);

				node_ctxt_.cur_binop = binaryOperation::PASS;
				node_ctxt_.dir = dir::UNKNOWNDIR;
				return value_;
			}

			bool isAssignable(const std::unique_ptr<IASTExpr> &op) const;
			bool shouldOpReturnFloat(const binaryOperation& op) const; // used for operations that return float instead of normal values
			std::size_t getExprResultType(const binaryOperation& op, std::size_t& lhs, const std::size_t& rhs);

			struct nodecontext
			{
				dir dir;
				binaryOperation cur_binop;
			} node_ctxt_;
	};

}

