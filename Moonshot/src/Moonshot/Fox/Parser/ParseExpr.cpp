
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

std::unique_ptr<ASTExpr> Parser::parseExpr(const char & priority)
{
	auto rtr = std::make_unique<ASTExpr>(parse::optype::PASS);
	std::unique_ptr<ASTExpr> left;

	if (priority > 0)
		left = parseExpr(priority - 1);	// Go down in the priority chain.
	else 
		left = parseTerm();	// We are at the lowest point ? Parse the term then !

	if (!left)					// Check if it was found/parsed correctly. If not, return a null node.
		return NULL_UNIPTR(ASTExpr);

	rtr->makeChild(parse::direction::LEFT, left);	// Make Left the left child of the return node !
	
	while (true)
	{
		parse::optype op;
		bool matchResult;
		std::tie(matchResult, op) = matchBinaryOp(priority);
		if (!matchResult) // No operator found : break.
			break;
		std::unique_ptr<ASTExpr> right;

		if (priority > 0)
			right = parseExpr(priority - 1);
		else
			right = parseTerm();
		// Check for validity : we need a term. if we don't have one, we have an error !
		if (!right)
		{
			errorExpected("Expected a term");
			break;
		}
		// Add the node to the tree.
		if (rtr->op_ == parse::optype::PASS) // No right node ? Append directly to rtr.
			rtr->op_ = op;
		else	// Already has one ? create a new node and make rtr its left child.
			oneUpNode(rtr, op);
		rtr->makeChild(parse::direction::RIGHT, right);
	}

	auto simple = rtr->getSimple();
	if (simple)
		return simple;

	return rtr;
}

std::unique_ptr<ASTExpr> Parser::parseTerm()
{
	// Search for a unary operator
	bool uopResult = false, mustcastResult = false;
	parse::types casttype = parse::types::NOCAST;
	parse::optype uopOp;
	std::tie(uopResult, uopOp) = matchUnaryOp();
	
	// Search for a value
	auto val = parseValue();
	if (!val)
		return NULL_UNIPTR(ASTExpr); // No value here? Return a null node.

	// Search for a cast: "as" <type>
	if (matchKeyword(lex::TC_AS))
	{
		std::tie(mustcastResult, casttype) = matchTypeKw();
		if (!mustcastResult)
			errorExpected("Expected a type keyword after \"as\"");
	}
	// Apply the unary operator (if found) to the node.
	if (uopResult)
	{
		if (val->op_ != parse::PASS) // If we already have an operation
			val = oneUpNode(val, uopOp); // One up the node and set the new parent's operation to uopOp
		else
			val->op_ = uopOp; // Else, just set the op_ to uopOp;
	}
	// Apply the cast (if found) to the node
	if (mustcastResult)
		val->setMustCast(casttype);
	return val;
}

std::unique_ptr<ASTExpr> Parser::parseValue()
{
	auto cur = getToken();
	// if the token is invalid, return directly a null node
	if (!cur.isValid())
		return NULL_UNIPTR(ASTExpr);
	// = <const>
	if (cur.type == lex::tokentype::TT_VALUE) // if we have a value, return it packed in a ASTValue
	{
		pos_ += 1;
		return std::make_unique<ASTValue>(cur);
	}
	// = '(' <expr> ')'
	if (matchSign(lex::B_ROUND_OPEN))
	{
		auto expr = parseExpr(); // Parse the expression inside
		cur = getToken();		// update current token
		// check validity of the parsed expression
		if(!expr)
		{
			errorExpected("Expected an expression after opening a bracket.");
			return NULL_UNIPTR(ASTExpr);
		}
		// retrieve the closing bracket, throw an error if we don't have one. 
		if (!matchSign(lex::B_ROUND_CLOSE))
		{
			errorExpected("Expected a closing bracket after expression");
			return NULL_UNIPTR(ASTExpr);
		}
		return expr;
	}
	// TO DO :
	// <callable>
	errorUnexpected();
	return NULL_UNIPTR(ASTExpr);
}

std::pair<bool, parse::optype> Moonshot::Parser::matchUnaryOp()
{
	// Avoid long & verbose lines.
	using namespace lex;
	using namespace parse;

	auto cur = getToken();
	if (!cur.isValid() || (cur.type != lex::tokentype::TT_SIGN))
		return { false, parse::optype::PASS };

	if (cur.sign_type == lex::signs::P_EXCL_MARK)
	{
		pos_ += 1;
		return { true, parse::optype::INVERT };
	}
	if (cur.sign_type == lex::signs::S_MINUS)
	{
		pos_ += 1;
		return { true, parse::optype::NEGATE};
	}

	return { false, parse::optype::PASS };
}

std::pair<bool, parse::optype> Parser::matchBinaryOp(const char & priority)
{
	// Avoid long & verbose lines.
	using namespace lex;
	using namespace parse;

	auto cur = getToken();
	auto pk = getToken(pos_ + 1);
	// Check current token validity
	if (!cur.isValid() || (cur.type != tokentype::TT_SIGN))
		return { false, optype::DEFAULT };
	pos_ += 1; // We already increment once here in prevision of a matched operator. We'll decrease before returning the result if nothing was found, of course.

	switch (priority)
	{
		case 0: // * / %
			if (cur.sign_type == signs::S_ASTERISK)
				return { true, optype::MUL };
			if (cur.sign_type == signs::S_SLASH)
				return { true, optype::DIV };
			if (cur.sign_type == signs::S_PERCENT)
				return { true, optype::MOD };
			if (cur.sign_type == signs::S_EXP)
				return { true, optype::EXP };
			break;
		case 1: // + -
			if (cur.sign_type == signs::S_PLUS)
				return { true, optype::ADD };
			if (cur.sign_type == signs::S_MINUS)
				return { true, optype::MINUS };
			break;
		case 2: // > >= < <=
			if (cur.sign_type == signs::S_LESS_THAN)
			{
				if (pk.isValid() && (pk.sign_type == signs::S_EQUAL))
				{
					pos_ += 1;
					return { true, optype::LESS_OR_EQUAL };
				}
				return { true, optype::LESS_THAN };
			}
			if (cur.sign_type == signs::S_GREATER_THAN)
			{
				if (pk.isValid() && (pk.sign_type == signs::S_EQUAL))
				{
					pos_ += 1;
					return { true, optype::GREATER_OR_EQUAL };
				}
				return { true, optype::GREATER_THAN };
			}
			break;
		case 3:	// == !=
			if (pk.isValid() && (pk.sign_type == signs::S_EQUAL))
			{
				if (cur.sign_type == signs::S_EQUAL)
				{
					pos_ += 1;
					return { true,optype::EQUAL };
				}
				if (cur.sign_type == signs::P_EXCL_MARK)
				{
					pos_ += 1;
					return { true,optype::NOTEQUAL };
				}
			}
			break;
		case 4:
			if (pk.isValid() && (pk.sign_type == signs::S_AND) && (cur.sign_type == signs::S_AND))
			{
				pos_ += 1;
				return { true,optype::AND };
			}
			break;
		case 5:
			if (pk.isValid() && (pk.sign_type == signs::S_VBAR) && (cur.sign_type == signs::S_VBAR))
			{
				pos_ += 1;
				return { true,optype::OR };
			}
			break;
		default:
			E_CRITICAL("Requested to match a Binary Operator with a non-existent priority");
			break;
	}
	pos_ -= 1;	// We did not find anything, decrement & return.
	return { false, optype::DEFAULT };
}
