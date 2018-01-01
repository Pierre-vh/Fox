////------------------------------------------------------////
// This file is a part of The Moonshot Project.				//
// See LICENSE.txt for license info.						//
// File : ParseExpr.cpp										//
// Author : Pierre van Houtryve								//
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			//
// This file implements expressions related methods (rules)	//
////------------------------------------------------------////

#include "Parser.h"

using namespace Moonshot;
using namespace fv_util;

std::unique_ptr<ASTExpr> Parser::parseExpr(const char & priority)
{
	auto rtr = std::make_unique<ASTExpr>(parse::optype::PASS);
	std::unique_ptr<ASTExpr> first = 0, last = 0;

	if (priority > 0)
		first = parseExpr(priority - 1);	// Go down in the priority chain.
	else 
		first = parseTerm();	// We are at the lowest point ? Parse the term then !

	if (!first)					// Check if it was found/parsed correctly. If not, return a null node.
		return NULL_UNIPTR(ASTExpr);

	rtr->makeChild(parse::direction::LEFT, first);	// Make Left the left child of the return node !
	while (true)
	{
		parse::optype op;
		bool matchResult;
		std::tie(matchResult, op) = matchBinaryOp(priority);
		if (!matchResult) // No operator found : break.
			break;
		std::unique_ptr<ASTExpr> second;

		if (priority > 0)
			second = parseExpr(priority - 1);
		else
			second = parseTerm();
		// Check for validity : we need a term. if we don't have one, we have an error !
		if (!second)
		{
			errorExpected("Expected a term");
			break;
		}
		// Add the node to the tree but in different ways, depending on left or right assoc.
		if (!parse::isRightAssoc(op))		// Left associative operator
		{
			if (rtr->op_ == parse::optype::PASS)
				rtr->op_ = op;
			else	// Already has one ? create a new node and make rtr its left child.
				rtr = oneUpNode(rtr, op);
			rtr->makeChild(parse::direction::RIGHT, second);
		}									
		// I Have to admit, this bit might have poor performance if you start to go really deep, like 2^2^2^2^2^2^2^2^2^2^2^2^2^2^2^2^2^2^2^2^2^2^2^2^2.. and so on, but who would do this anyway, right..right ?
		// To be serious:there isn't really another way of doing this,except if I change completly my parsing algorithm to do both right to left and left to right parsing.
		// OR if i implement the shunting yard algo, and that's even harder to do with my current system.
		// Which is complicated, and not really useful for now, as <expr> is the only rule
		// that might need right assoc.
		else								// right associative nodes
		{
			// There's a member variable, last_ that will hold the last parsed "second" variable.
			
			// First "loop" check.
			if (rtr->op_ == parse::PASS)
				rtr->op_ = op;			

			if (!last) // Last is empty
				last = std::move(second); // Set last_ to second.
			else
			{
				_ASSERT(op != parse::PASS);
				auto newnode_op = std::make_unique<ASTExpr>(op); // Create a node with the op
				newnode_op->makeChild(parse::LEFT, last); // Set last_ as left child.
				last = std::move(second);// Set second as last
				// Append newnode_op to rtr
				rtr->makeChildOfDeepestNode(parse::RIGHT, newnode_op);
			}
		}
	}
	if (last) // Last isn't empty -> make it the left child of our last node.
	{
		rtr->makeChildOfDeepestNode(parse::RIGHT, last);
		last = 0;
	}
	// When we have simple node (PASS optype with only a value/expr as left child), we simplify it(only return the left child)
	auto simple = rtr->getSimple();
	if (simple)
		return simple;
	return rtr;
}

std::unique_ptr<ASTExpr> Parser::parseTerm()
{
	// Search for a unary operator
	bool uopResult = false;
	std::size_t casttype = invalid_index;
	parse::optype uopOp;
	std::tie(uopResult, uopOp) = matchUnaryOp();
	
	// Search for a value
	auto val = parseValue();
	if (!val)
		return NULL_UNIPTR(ASTExpr); // No value here? Return a null node.

	// Search for a cast: "as" <type>
	if (matchKeyword(lex::TC_AS))
	{
		casttype = matchTypeKw();
		if (casttype == invalid_index)
			errorExpected("Expected a type keyword after \"as\"");
	}
	// Apply the unary operator (if found) to the node.
	if (uopResult)
	{
		if ((val->op_ != parse::PASS) || (typeid(*val) == typeid(*std::make_unique<ASTRawValue>()))) // If we already have an operation OR the node is a value
			val = oneUpNode(val, uopOp); // One up the node and set the new parent's operation to uopOp
		else
			val->op_ = uopOp; // Else, just set the op_ to uopOp;
	}
	// Apply the cast (if found) to the node
	if (casttype != invalid_index)
	{
		val = oneUpNode(val, parse::optype::CAST); // Create a "cast" node
		val->totype_ = casttype;
	}

	return val;
}

std::unique_ptr<ASTExpr> Parser::parseValue()
{
	// if the token is invalid, return directly a null node

	// = <const>
	auto matchValue_result = matchValue();
	if (matchValue_result.first) // if we have a value, return it packed in a ASTRawValue
		return std::make_unique<ASTRawValue>(matchValue_result.second);
	// = '(' <expr> ')'
	else if (matchSign(lex::B_ROUND_OPEN))
	{
		auto expr = parseExpr(); // Parse the expression inside
		if(!expr) // check validity of the parsed expression
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
	else if (auto node = parseCallable()) // Callable?
		return node;
	// TO DO :
	// f_call
	return NULL_UNIPTR(ASTExpr);
}

std::unique_ptr<ASTExpr> Parser::parseCallable()
{
	// = <id>
	auto result = matchID();
	if (result.first)
		return std::make_unique<ASTVarCall>(result.second);
	return NULL_UNIPTR(ASTExpr);
}

std::unique_ptr<ASTExpr> Parser::oneUpNode(std::unique_ptr<ASTExpr>& node, const parse::optype & op)
{
	auto newnode = std::make_unique<ASTExpr>(op);
	newnode->makeChild(parse::LEFT, node);
	return newnode;
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
		return { true, parse::optype::LOGICNOT };
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
		return { false, optype::PASS };
	pos_ += 1; // We already increment once here in prevision of a matched operator. We'll decrease before returning the result if nothing was found, of course.

	switch (priority)
	{
		case 0:
			if (cur.sign_type == signs::S_EXP)
				return { true, optype::EXP };
			break;
		case 1: // * / %
			if (cur.sign_type == signs::S_ASTERISK)
				return { true, optype::MUL };
			if (cur.sign_type == signs::S_SLASH)
				return { true, optype::DIV };
			if (cur.sign_type == signs::S_PERCENT)
				return { true, optype::MOD };
			break;
		case 2: // + -
			if (cur.sign_type == signs::S_PLUS)
				return { true, optype::ADD };
			if (cur.sign_type == signs::S_MINUS)
				return { true, optype::MINUS };
			break;
		case 3: // > >= < <=
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
		case 4:	// == !=
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
		case 5:
			if (pk.isValid() && (pk.sign_type == signs::S_AND) && (cur.sign_type == signs::S_AND))
			{
				pos_ += 1;
				return { true,optype::AND };
			}
			break;
		case 6:
			if (pk.isValid() && (pk.sign_type == signs::S_VBAR) && (cur.sign_type == signs::S_VBAR))
			{
				pos_ += 1;
				return { true,optype::OR };
			}
			break;
		case 7:
			if ((cur.sign_type == signs::S_EQUAL)
				&&
				!(pk.isValid() && (pk.sign_type == signs::S_EQUAL))) // Refuse if op is ==
				return { true,optype::ASSIGN };
			break;
		default:
			E_CRITICAL("Requested to match a Binary Operator with a non-existent priority");
			break;
	}
	pos_ -= 1;	// We did not find anything, decrement & return.
	return { false, optype::PASS };
}
