
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

#include "ASTNode.h"
#include "../../Lexer/Token.h"
#include "../../../Common/Errors/Errors.h"

#include <iostream> // std::cout for debug purposes
#include <memory>	// std::unique_ptr
#include <sstream> // std::stringstream
#include <variant>	// std::variant

namespace Moonshot	
{
	namespace parse
	{
		enum optype
		{
			DEFAULT,
			PASS,			// Just "pass" (return the value in L)

			AND,
			OR,

			ADD,
			MINUS,
			MUL,
			DIV,

			LESS_OR_EQUAL,
			GREATER_OR_EQUAL,
			LESS_THAN,
			GREATER_THAN,
			EQUAL,
			NOTEQUAL
		};
		enum direction
		{
			LEFT,RIGHT
		};
	}
	class ASTExpr : public ASTNode
	{
		public:
			ASTExpr();
			ASTExpr(const parse::optype &opt);
			~ASTExpr();

			virtual void showTree();
			void makeChild(const parse::direction &d,std::unique_ptr<ASTExpr> &node);
			void setOpType(const parse::optype &nop);
			bool hasNode(const parse::direction &d) const;	// If the node posseses a left/right child, it will return true
			
			std::unique_ptr<ASTExpr> getSimple();	// If there is no right node and the optype is "pass", this will move and return the left node (because this means that this "expr" node is useless.
			// TODO find a way to return the node without moving it? or rename the function.
			parse::optype getOpType() const;
		private:
			std::unique_ptr<ASTExpr> left_ = 0, right_ = 0;
			parse::optype op_ = parse::DEFAULT;
	};
	class ASTValue : public ASTExpr
	{
		public:
			ASTValue();
			ASTValue(const token &t);
			~ASTValue();
			void showTree();
			std::string str;

			// Delete useless methods
			void makeChild(const parse::direction &d, std::unique_ptr<ASTExpr> &node) = delete;
			bool hasNode(const parse::direction &d) const = delete;
			std::unique_ptr<ASTExpr> getNode(const parse::direction &d) = delete;
			parse::optype getOpType() const = delete;
	protected:
			std::variant<int, float, char, std::string, bool> val_;
	};
}

