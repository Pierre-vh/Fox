
/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : This file implements the expression related rules.

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

#include "Parser.h"

using namespace Moonshot;

ASTNode * Parser::matchExpr()
{
	ASTExpr * rtr = new ASTExpr(parse::PASS);		// The node to return.
	if(ASTNode * tmp = matchTerm())
	if (ASTExpr * L = dynamic_cast<ASTExpr*>(tmp))
	{
		rtr->left = L;
		// Match (second_op term) until there is none left
		while (1)
		{
			bool flag;
			parse::optype opt;
			std::tie(flag, opt) = parseSecondOp();
			if (!flag)
				break;
			// If the flag is true, we need to match the whole construction (second_op term)
			if (ASTExpr * R = dynamic_cast<ASTExpr*>(matchTerm()))
			{
				if (rtr->op == parse::PASS)	// convert a node with one child 
				{
					rtr->op = opt;
					rtr->right = R;
				}
				else // the node already has an operation
				{
					// Change the tree to have the past on the left, and the newly parsed one on the right
					ASTExpr * nroot = new ASTExpr(opt);
					nroot->left = rtr;
					// change rtr
					rtr = nroot;
					// set the other term to be the right child
					rtr->right = R;
				}
			}
			else
			{
				std::stringstream ss;
				ss << "Unexpected token \"" << getToken().str << "\" in position " << getToken().pos.asText() << std::endl;
				E_ERROR(ss.str())
			}
		}
	}
	else
	{
		E_LOG("Nothing was found.")
		return 0;
	}
	return rtr;
}

ASTNode * Parser::matchTerm()
{
	token t = getToken();
	if (t.type == lex::TT_VALUE && t.val_type == lex::VAL_INTEGER)
	{
		pos_ += 1;
		ASTExpr * p = new ASTExpr(parse::VALUE);
		return p;
	}
	else
	{
		std::cout << pos_ << "." << t.showFormattedTokenData();
		std::cin.get();
	}
	return 0;
}

ASTNode * Parser::matchFactor()
{
	return nullptr;
}

ASTNode * Parser::matchValue()
{
	return nullptr;
}
std::pair<bool, parse::optype> Moonshot::Parser::parseSecondOp()
{
	token c = getToken();
	if (c.sign_type == lex::S_PLUS)
	{
		std::cout << "Parsed +";
		pos_ += 1;
		return { true , parse::optype::PLUS };
	}
	else if (c.sign_type == lex::S_MINUS)
	{
		pos_ += 2;
		return { true , parse::optype::MINUS };
	}
	return parseCondJoinOp();	// Else, we let the matchCondJoinOp function handle the rest.
}
std::pair<bool, parse::optype> Moonshot::Parser::parseCondJoinOp()
{
	token c = getToken();
	token pk = getToken(pos_ + 1);
	if ((c.sign_type == lex::S_AND) && (pk.sign_type == lex::S_AND))
	{
		pos_ += 2;									// Increment position
		return { true, parse::optype::JOIN_AND };
	}
	else if ((c.sign_type == lex::S_VBAR) && (pk.sign_type == lex::S_VBAR))
	{
		pos_ += 2;									// Increment position
		return { true, parse::optype::JOIN_OR };
	}
	return { false,parse::optype::DEFAULT };
}
