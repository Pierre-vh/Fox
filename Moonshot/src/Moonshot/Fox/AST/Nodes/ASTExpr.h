
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

#include "../../AST/Visitor/IVisitor.h"
#include "../../../Common/Errors/Errors.h"		// error reporting
#include "../../../Common/FValue/FValue.h"		// FValue alias

#include <iostream> // std::cout for debug purposes
#include <memory>	// std::unique_ptr
#include <sstream> // std::stringstream
#include <variant>	// std::variant

#define VISIT_THIS vis->visit(this);

namespace Moonshot	
{
	struct ASTExpr : public IASTNode
	{
		public:
			ASTExpr();
			ASTExpr(const parse::optype &opt);
			~ASTExpr();

			void makeChild(const parse::direction &d,std::unique_ptr<ASTExpr> &node); // make (node) a child of this.
			bool hasNode(const parse::direction &d) const;	// If the node posseses a left/right child, it will return true
			
			void setReturnType(const parse::types &casttype); // set totype_
			parse::types getToType() const;					// return totype_

			std::unique_ptr<ASTExpr> getSimple();			// If there is no right node and the optype is "pass", this will move and return the left node (because this means that this "expr" node is useless.)

			// Accept
			virtual FVal accept(IVisitor *vis) override;

			// NODE DATA
			// Expression nodes hold 4 values :
			// totype_ : the return type of the node
			// op_ : the operation the node should perform
			// left_ & right_ -> pointers to its children
			parse::types totype_ = parse::types::NOTYPE;	// By default, don't cast. If this is different than "NONE", then we must cast the result to the desired type.
			parse::optype op_ = parse::DEFAULT;
			std::unique_ptr<ASTExpr> left_ = 0, right_ = 0;


	};
	struct ASTValue : public ASTExpr
	{
		public:
			ASTValue();
			ASTValue(const token &t);
			~ASTValue();

			FVal accept(IVisitor *vis) override;

			// NODE DATA
			// Value node holds 1 value : (inherited ones are never called and ignored.)
			// val_ -> std::variant that holds the data of the node
			FVal val_;
	};
}

