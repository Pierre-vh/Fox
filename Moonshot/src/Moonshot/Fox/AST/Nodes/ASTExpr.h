
/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : A Node class for expressions.

*************************************************************
MIT License

Copyright (c) 2017 Pierre van Houtryve

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*************************************************************/

#pragma once

#include "IASTNode.h"							// Abstract class that every node must inherit from.
#include "../../Lexer/Token.h"					// Lexer's token
#include "../../Util/Enums.h"					// enums

#include "../../../Common/Errors/Errors.h"		// error reporting
#include "../../../Common/FValue/FValue.h"		// FValue alias

#include <algorithm> // swap
#include <iostream> // std::cout for debug purposes
#include <memory>	// std::unique_ptr
#include <sstream> // std::stringstream
#include <variant>	// std::variant
#include <functional> // std:::function

namespace Moonshot	
{
	struct ASTExpr : public IASTNode
	{
		public:
			ASTExpr();
			ASTExpr(const parse::optype &opt);
			~ASTExpr();

			void makeChild(const parse::direction &d,std::unique_ptr<ASTExpr> &node); // make (node) a child of this.
			void makeChildOfDeepestNode(const parse::direction &d, std::unique_ptr<ASTExpr> &node); // Make (node) a child of the deepest left/right path of our node. (continue until left/right = 0, then makechild.)
			
			ASTExpr* getDeepestNode(const parse::direction &d); // returns the deepest left/right child (always go left/right from first node)

			bool hasNode(const parse::direction &d) const;	// If the node posseses a left/right child, it will return true
			void setReturnType(const std::size_t &casttype); // set totype_
			std::size_t getToType() const;					// return totype_

			void swapChildren();

			std::unique_ptr<ASTExpr> getSimple();			// If there is no right node and the optype is "pass", this will move and return the left node (because this means that this "expr" node is useless.)

			// Accept
			virtual void accept(IVisitor *vis) override;
			virtual FVal accept(IRTVisitor *vis) override;
			// NODE DATA
			// Expression nodes hold 4 values :
			// totype_ : the return type of the node
			// op_ : the operation the node should perform
			// left_ & right_ -> pointers to its children
			std::size_t totype_ = invalid_index;	// By default, don't cast (-1). If this is different , then we must cast the result to the desired type.
			parse::optype op_ = parse::PASS;
			std::unique_ptr<ASTExpr> left_ = 0, right_ = 0;
	};
	struct ASTValue : public ASTExpr
	{
		public:
			ASTValue();
			ASTValue(const token &t);
			~ASTValue();

			void accept(IVisitor *vis) override;
			FVal accept(IRTVisitor *vis) override;
			// NODE DATA
			// Value node holds 1 value : (inherited ones are never called and ignored.)
			// val_ -> std::variant that holds the data of the node
			FVal val_;
	};
}

