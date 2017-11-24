
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

std::unique_ptr<ASTExpr> Parser::parseExpr()
{
	auto rtr = std::make_unique < ASTExpr>(parse::PASS);	// By default just set the operation to PASS (no op). We'll edit it later if other groups are found !
	auto left = parseTerm();
	// If a term wasn't matched, just return false
	if (!left)
		return NULL_UNIPTR(ASTExpr);
	// Set as left child.
	rtr->makeChild(parse::direction::LEFT, left);
	// Match (<second_op> <term>) again and again until we can't find it anymore.
	while (1)
	{
		parse::optype op;
		bool matchResult;
		std::tie(matchResult, op) = matchSecondOp();
		if (!matchResult)
			break;			// Found nothing interesting : break !
		std::unique_ptr<ASTExpr> right = parseTerm();
		if (!right)
		{
			errorExpected("Expected a term ");
			break;
		}
		if (rtr->getOpType() == parse::PASS)
		{
			rtr->setOpType(op);
			rtr->makeChild(parse::RIGHT, right);
		}
		else
		{
			rtr = oneUpNode(rtr, op);
			rtr->makeChild(parse::RIGHT, right);
		}
	}
	auto simple = rtr->getSimple();
	if (simple)
		return simple;
	return rtr;
}

std::unique_ptr<ASTExpr> Parser::parseTerm()
{
	auto rtr = std::make_unique < ASTExpr>(parse::PASS);	// By default just set the operation to PASS (no op). We'll edit it later if other groups are found !
	auto left = parseFactor();
	// If a term wasn't matched, just return false
	if (!left)
		return NULL_UNIPTR(ASTExpr);
	// Set as left child.
	rtr->makeChild(parse::direction::LEFT, left);	// Match (<second_op> <term>) again and again until we can't find it anymore.
	while (1)
	{
		parse::optype op;
		bool matchResult;
		std::tie(matchResult, op) = matchPriorOp();
		if (!matchResult)
			break;			// Found nothing interesting : break !
		std::unique_ptr<ASTExpr> right = parseFactor();
		if (!right)
		{
			errorExpected("BExpected a term ");
			break;
		}
		if (rtr->getOpType() == parse::PASS)
		{
			rtr->setOpType(op);
			rtr->makeChild(parse::RIGHT, right);
		}
		else
		{
			rtr = oneUpNode(rtr, op);
			rtr->makeChild(parse::RIGHT, right);
		}
	}
	auto simple = rtr->getSimple();
	if (simple)
		return simple;
	return rtr; 
}

std::unique_ptr<ASTExpr> Parser::parseFactor()
{
	pos_ += 1;
	return std::make_unique<ASTValue>(token("3.3"));
}

std::unique_ptr<ASTExpr> Parser::parseValue()
{
	return NULL_UNIPTR(ASTExpr);
}
