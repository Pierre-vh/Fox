////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTExpr.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// AST nodes for expressions											
////------------------------------------------------------////

#pragma once

#include "IASTExpr.hpp"							
#include "Moonshot/Fox/Common/Operators.hpp"			// enums
#include "Moonshot/Common/Types/Types.hpp"		// FoxValue
#include <memory>

namespace Moonshot	
{
	// Represents a binary expression
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

	// Represents a unary expression
	struct ASTUnaryExpr : public IASTExpr
	{
		public: 
			ASTUnaryExpr() = default;
			ASTUnaryExpr(const unaryOperator& opt);
			virtual void accept(IVisitor& vis) override;

			std::unique_ptr<IASTExpr> child_;
			unaryOperator op_ = unaryOperator::DEFAULT;
	};

	// Represents a cast expression <expr> "as" <type>
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

	// Represents a literal
	struct ASTLiteralExpr : public IASTExpr 
	{
		public:
			ASTLiteralExpr() = default;
			ASTLiteralExpr(const FoxValue &fv);

			void accept(IVisitor& vis) override;

			FoxValue val_;
	};

	// Represents a reference to a declaration (namespace,variable,function) -> it's an identifier!
	struct ASTDeclRefExpr : public IASTExpr 
	{
		public:
			ASTDeclRefExpr() = default;
			ASTDeclRefExpr(const std::string& vname);

			void accept(IVisitor& vis) override;
			
			std::string declname_ = "";
	};
}

