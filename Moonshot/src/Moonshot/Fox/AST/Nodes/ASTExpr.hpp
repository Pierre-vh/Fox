////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTExpr.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// AST nodes for expressions											
////------------------------------------------------------////

#pragma once

#include "IASTStmt.hpp"							
#include "../../Util/Enums.hpp"					// enums
#include "../../../Common/Types/Types.hpp"		// FVal

namespace Moonshot	
{
	struct IASTExpr : public IASTStmt
	{
		public:
			IASTExpr() = default;
			inline virtual ~IASTExpr() = 0 {}

			std::size_t resultType_ = 0; // The planified result type of the expression after execution. this is set by the typechecker.
	};
	struct ASTBinaryExpr : public IASTExpr
	{
		public:
			ASTBinaryExpr() = default;
			ASTBinaryExpr(const binaryOperation &opt);


			std::unique_ptr<IASTExpr> left_, right_;
			binaryOperation op_ = binaryOperation::PASS;
			std::size_t resultType_ = 0; 

			virtual void accept(IVisitor& vis) override;
			std::unique_ptr<IASTExpr> getSimple();	// If there is no right node and the optype is "pass", this will move and return the left node (because this means that this "expr" node is useless.)

			void setChild(const dir &d, std::unique_ptr<IASTExpr> &node); // make (node) a child of this.
			void makeChildOfDeepestNode(const dir &d, std::unique_ptr<IASTExpr> &node); // Make (node) a child of the deepest left/right path of our node. (continue until left/right = 0, then makechild.)
	};
	struct ASTUnaryExpr : public IASTExpr
	{
		public: 
			ASTUnaryExpr() = default;
			ASTUnaryExpr(const unaryOperation& opt);
			virtual void accept(IVisitor& vis) override;

			std::unique_ptr<IASTExpr> child_;
			unaryOperation op_ = unaryOperation::DEFAULT;
			std::size_t resultType_ = 0; 
	};
	struct ASTCastExpr : public IASTExpr
	{
		public:
			ASTCastExpr() = default;
			ASTCastExpr(std::size_t castGoal);
			virtual void accept(IVisitor& vis) override;

			std::unique_ptr<IASTExpr> child_;
			std::size_t resultType_ = 0;

			void setCastGoal(const std::size_t& ncg);
			std::size_t getCastGoal() const; 
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

