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
#include "Moonshot/Common/Types/Types.hpp"		// FVal
#include <memory>

namespace Moonshot	
{

	struct ASTBinaryExpr : public IASTExpr
	{
		public:
			ASTBinaryExpr() = default;
			ASTBinaryExpr(const binaryOperator &opt);


			std::unique_ptr<IASTExpr> left_, right_;
			binaryOperator op_ = binaryOperator::PASS;

			virtual void accept(IVisitor& vis) override;
			std::unique_ptr<IASTExpr> getSimple();	// If there is no right node and the optype is "pass", this will move and return the left node (because this means that this "expr" node is useless.)
	};

	struct ASTUnaryExpr : public IASTExpr
	{
		public: 
			ASTUnaryExpr() = default;
			ASTUnaryExpr(const unaryOperator& opt);
			virtual void accept(IVisitor& vis) override;

			std::unique_ptr<IASTExpr> child_;
			unaryOperator op_ = unaryOperator::DEFAULT;
	};

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

	struct ASTLiteral : public IASTExpr 
	{
		public:
			ASTLiteral() = default;
			ASTLiteral(const FVal &fv);

			void accept(IVisitor& vis) override;

			FVal val_;
	};

	struct ASTVarCall : public IASTExpr 
	{
		public:
			ASTVarCall() = default;
			ASTVarCall(const std::string& vname);

			void accept(IVisitor& vis) override;
			
			//The varattr, which serves as a "reference" to the variable stored.
			std::string varname_ = "";
	};
}

