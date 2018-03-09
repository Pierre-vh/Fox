////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTExpr.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Declares the IASTExpr interface as well as derived nodes. 
////------------------------------------------------------////

#pragma once

#include "ASTStmt.hpp"
#include "Moonshot/Fox/AST/IVisitor.hpp"
#include "Moonshot/Fox/Common/Operators.hpp"			// enums
#include "Moonshot/Common/Types/Types.hpp"		// FoxValue
#include <memory>

namespace Moonshot	
{
	// base expression interface
	struct IASTExpr : public IASTStmt
	{
		public:
			IASTExpr() = default;
			inline virtual ~IASTExpr() = 0 {}
			virtual void accept(IVisitor& vis) = 0;
			FoxType resultType_ = 0; // The planified result type of the expression after execution. this is set by the typechecker.
	};

	// Binary Expressions
	struct ASTBinaryExpr : public IASTExpr
	{
		public:
			ASTBinaryExpr() = default;
			ASTBinaryExpr(const binaryOperator &opt);


			std::unique_ptr<IASTExpr> left_, right_;
			binaryOperator op_ = binaryOperator::PASS;

			virtual void accept(IVisitor& vis) override;
			std::unique_ptr<IASTExpr> getSimple();	// If there is no right node and the optype is "pass", this will move and return the left node 
	};

	// Unary Expressions
	struct ASTUnaryExpr : public IASTExpr
	{
		public: 
			ASTUnaryExpr() = default;
			ASTUnaryExpr(const unaryOperator& opt);
			virtual void accept(IVisitor& vis) override;

			std::unique_ptr<IASTExpr> child_;
			unaryOperator op_ = unaryOperator::DEFAULT;
	};

	// Explicit Cast Expressions
	struct ASTCastExpr : public IASTExpr
	{
		public:
			ASTCastExpr() = default;
			ASTCastExpr(std::size_t castGoal);
			virtual void accept(IVisitor& vis) override;

			std::unique_ptr<IASTExpr> child_;

			void setCastGoal(const FoxType& ncg);
			FoxType getCastGoal() const; 
	};

	// Literals
	struct ASTLiteralExpr : public IASTExpr 
	{
		public:
			ASTLiteralExpr() = default;
			ASTLiteralExpr(const FoxValue &fv);

			void accept(IVisitor& vis) override;

			FoxValue val_;
	};

	/*
		Note: the AST needs to be adapted for theses 2 nodes. std::strings should be replaced by a pointer to a IASTIdentifier (or find a better name)
		IASTIdentifier has 2 children : Unresolved ID and resolved ID. Unresolved ID are the raw strings produced by the parsing, and they're replaced by the 
		Resolver in the Semantic analysis phase by Resolved IDs. Resolved IDs contain a pointer to an entry in the master symbols table.
	*/

	// Represents a reference to a declaration (namespace,variable,function) -> it's an identifier!
	struct ASTDeclRefExpr : public IASTExpr 
	{
		public:
			ASTDeclRefExpr() = default;
			ASTDeclRefExpr(const std::string& vname);

			void accept(IVisitor& vis) override;
			
			std::string declname_ = "";
	};

	// Represents a member access operation on a namespace, struct or anything
	// expr is the expression that is being accessed, id_ is the identifier to search.
	struct ASTMemberAccessExpr : public IASTExpr
	{
		ASTMemberAccessExpr() = default;

		// the expression that is being accessed
		std::unique_ptr<IASTExpr> expr_;
		// the identifier to search inside the namespace/object/etc.
		std::string id_;
	};
}

