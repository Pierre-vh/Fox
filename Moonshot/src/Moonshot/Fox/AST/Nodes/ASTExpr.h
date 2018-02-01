////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTExpr.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// AST nodes for expressions											
////------------------------------------------------------////

#pragma once

#include "IASTStmt.h"							// Abstract class that every node must inherit from.
#include "../../Lexer/Token.h"					// Lexer's Token
#include "../../Util/Enums.h"					// enums
#include "../../../Common/Types/Types.h"		// FValue alias
#include <algorithm> // swap
#include <iostream> // std::cout for debug purposes
#include <sstream> // std::stringstream

namespace Moonshot	
{
	struct ASTExpr : public IASTStmt
	{
		public:
			ASTExpr() = default;
			ASTExpr(const operation &opt);
			~ASTExpr();

			void makeChild(const dir &d,std::unique_ptr<ASTExpr> &node); // make (node) a child of this.
			void makeChildOfDeepestNode(const dir &d, std::unique_ptr<ASTExpr> &node); // Make (node) a child of the deepest left/right path of our node. (continue until left/right = 0, then makechild.)
			
			bool hasNode(const dir &d) const;	// If the node posseses a left/right child, it will return true
			void setReturnType(const std::size_t &casttype); // set totype_
			std::size_t getToType() const;					// return totype_

			void swapChildren();

			std::unique_ptr<ASTExpr> getSimple();			// If there is no right node and the optype is "pass", this will move and return the left node (because this means that this "expr" node is useless.)

			// Accept
			virtual void accept(IVisitor& vis) override;
			// NODE DATA
			// Expression nodes hold 4 values :
			// totype_ : the return type of the node
			// op_ : the operation the node should perform
			// left_ & right_ -> pointers to its children
			std::size_t totype_ = fv_util::invalid_index;	// By default, don't cast (-1). If this is different , then we must cast the result to the desired type.
			operation op_ = operation::PASS;
			std::unique_ptr<ASTExpr> left_, right_;
	};
	struct ASTLiteral : public ASTExpr // Stores hard coded constants. 3+3 -> 3 are Hard coded constants.
	{
		public:
			ASTLiteral() = default;
			ASTLiteral(const Token &t);
			~ASTLiteral();

			void accept(IVisitor& vis) override;
			// NODE DATA
			// Value node holds 1 value : (inherited ones are never called and ignored.)
			// val_ -> std::variant that holds the data of the node
			FVal val_;
	};
	struct ASTVarCall : public ASTExpr // Store var calls : foo+3 -> foo is a var call;
	{
		public:
			ASTVarCall() = default;
			ASTVarCall(const std::string& vname);
			~ASTVarCall();

			void accept(IVisitor& vis) override;
			
			//The varattr, which serves as a "reference" to the variable stored.
			std::string varname_ = "";
	};
}

