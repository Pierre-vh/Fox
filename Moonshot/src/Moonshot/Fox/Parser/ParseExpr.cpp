////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ParseExpr.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
// This file implements expressions related methods (rules)	
////------------------------------------------------------////

#include "Parser.h"

using namespace Moonshot;
using namespace fv_util;


std::unique_ptr<IASTExpr> Parser::parseExpr(const char & priority)
{
	auto rtr = std::make_unique<ASTBinaryExpr>(binaryOperation::PASS);

	// change this to ASTBinaryExpr
	std::unique_ptr<IASTExpr> first = 0, tmp = 0;

	if (priority > 0)
		first = parseExpr(priority - 1);	// Go down in the priority chain.
	else 
		first = parseCastExpr();	// We are at the lowest point ? Parse the term then !

	if (!first)					// Check if it was found/parsed correctly. If not, return a null node.
		return nullptr;

	rtr->makeChild(dir::LEFT, first);	// Make Left the left child of the return node !
	while (true)
	{
		binaryOperation op;
		bool matchResult;
		std::tie(matchResult, op) = matchBinaryOp(priority);
		if (!matchResult) // No operator found : break.
			break;
		
		std::unique_ptr<IASTExpr> second;

		if (priority > 0) second = parseExpr(priority - 1);
		else second = parseCastExpr();

		// Check for validity : we need a term. if we don't have one, we have an error !
		if (!second)
		{
			errorExpected("Expected an expression after binary operator.");
			break;
		}
		// Add the node to the tree but in different ways, depending on left or right assoc.
		if (!isRightAssoc(op))		// Left associative operator
		{
			if (rtr->op_ == binaryOperation::PASS)
				rtr->op_ = op;
			else	// Already has one ? create a new node and make rtr its left child.
				rtr = oneUpNode(rtr, op);
			rtr->makeChild(dir::RIGHT, second);
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
			if (rtr->op_ == binaryOperation::PASS)
				rtr->op_ = op;			

			if (!tmp) // Last is empty
				tmp = std::move(second); // Set last_ to second.
			else
			{
				_ASSERT(op != binaryOperation::PASS);
				auto new_binop_node = std::make_unique<ASTBinaryExpr>(op);
				new_binop_node->left_ = std::move(tmp);
				std::unique_ptr<IASTExpr> newnode = std::move(new_binop_node); // Create a node with the op
				
				tmp = std::move(second);// Set second as tmp
				rtr->makeChildOfDeepestNode(dir::RIGHT, newnode);
			}
		}
	}
	if (tmp) // Last isn't empty -> make it the left child of our last node.
	{
		// if last.op != op::pass || tmp.right_ -> throw ast malformation error
		rtr->makeChildOfDeepestNode(dir::RIGHT, tmp);
		tmp = 0;
	}
	// When we have simple node (PASS operation with only a value/expr as left child), we simplify it(only return the left child)
	auto simple = rtr->getSimple();
	if (simple)
		return simple;
	return rtr;
}

std::unique_ptr<IASTExpr> Parser::parsePrefixExpr()
{
	bool uopResult = false;
	unaryOperation uopOp;
	std::size_t casttype = indexes::invalid_index;
	std::tie(uopResult, uopOp) = matchUnaryOp(); // If an unary op is matched, uopResult will be set to true and pos_ updated.
	if (uopResult)
	{
		if (auto node = parsePrefixExpr())
		{
			auto rtr = std::make_unique<ASTUnaryExpr>();
			rtr->op_ = uopOp;
			rtr->child_ = std::move(node);
			return rtr;
		}
		else
		{
			errorExpected("Expected an expression after unary operator in prefix expression.");
			return nullptr;
		}
	}
	else if (auto node = parseValue())
		return node;
	return nullptr;
}

std::unique_ptr<IASTExpr> Parser::parseCastExpr()
{
	if (auto node = parsePrefixExpr())
	{
		std::size_t casttype = indexes::invalid_index;
		// Search for a (optional) cast: "as" <type>
		if (matchKeyword(keywordType::TC_AS))
		{
			if ((casttype = matchTypeKw()) != indexes::invalid_index)
			{
				// If found, apply it to current node.
				auto rtr = std::make_unique<ASTCastExpr>();
				rtr->setCastGoal(casttype);
				rtr->child_ = std::move(node);
				return rtr;
			}
			else
			{
				// If error (invalid keyword found, etc.)
				errorExpected("Expected a type keyword after \"as\" in cast expression.");
				return nullptr;
			}
		}
		return node;
	}
	return nullptr;
}

std::unique_ptr<IASTExpr> Parser::parseValue()
{
	// if the Token is invalid, return directly a null node

	// = <const>
	auto matchValue_result = matchLiteral();
	if (matchValue_result.first) // if we have a value, return it packed in a ASTLiteral
		return std::make_unique<ASTLiteral>(matchValue_result.second);
	else if (auto node = parseCallable()) // Callable?
		return node;
	// = '(' <expr> ')'
	else if (matchSign(signType::B_ROUND_OPEN))
	{
		auto expr = parseExpr(); // Parse the expression inside
		if (!expr) // check validity of the parsed expression
		{
			errorExpected("Expected an expression after '('.");
			return nullptr;
		}
		// retrieve the closing bracket, throw an error if we don't have one. 
		if (!matchSign(signType::B_ROUND_CLOSE))
		{
			errorExpected("Expected a ')' after expression");
			return nullptr;
		}
		return expr;
	}
	// TO DO :
	// f_call
	return nullptr;
}

std::unique_ptr<IASTExpr> Parser::parseCallable()
{
	// = <id>
	auto result = matchID();
	if (result.first)
		return std::make_unique<ASTVarCall>(result.second);
	return nullptr;
}

std::unique_ptr<ASTBinaryExpr> Parser::oneUpNode(std::unique_ptr<ASTBinaryExpr>& node, const binaryOperation & op)
{
	auto newnode = std::make_unique<ASTBinaryExpr>(op);
	newnode->left_ = std::move(node);
	return newnode;
}

std::pair<bool, unaryOperation> Moonshot::Parser::matchUnaryOp()
{
	// Avoid long & verbose lines.
	

	auto cur = getToken();
	if (!cur.isValid() || (cur.type != tokenType::TT_SIGN))
		return { false, unaryOperation::DEFAULT };

	if (cur.sign_type == signType::P_EXCL_MARK)
	{
		pos_ += 1;
		return { true, unaryOperation::LOGICNOT };
	}
	if (cur.sign_type == signType::S_MINUS)
	{
		pos_ += 1;
		return { true, unaryOperation::NEGATE};
	}

	return { false, unaryOperation::DEFAULT };
}

std::pair<bool, binaryOperation> Parser::matchBinaryOp(const char & priority)
{
	// Avoid long & verbose lines.
	

	auto cur = getToken();
	auto pk = getToken(pos_ + 1);
	// Check current Token validity
	if (!cur.isValid() || (cur.type != tokenType::TT_SIGN))
		return { false, binaryOperation::PASS };
	pos_ += 1; // We already increment once here in prevision of a matched operator. We'll decrease before returning the result if nothing was found, of course.

	switch (priority)
	{
		case 0:
			if (cur.sign_type == signType::S_EXP)
				return { true, binaryOperation::EXP };
			break;
		case 1: // * / %
			if (cur.sign_type == signType::S_ASTERISK)
				return { true, binaryOperation::MUL };
			if (cur.sign_type == signType::S_SLASH)
				return { true, binaryOperation::DIV };
			if (cur.sign_type == signType::S_PERCENT)
				return { true, binaryOperation::MOD };
			break;
		case 2: // + -
			if (cur.sign_type == signType::S_PLUS)
				return { true, binaryOperation::ADD };
			if (cur.sign_type == signType::S_MINUS)
				return { true, binaryOperation::MINUS };
			break;
		case 3: // > >= < <=
			if (cur.sign_type == signType::S_LESS_THAN)
			{
				if (pk.isValid() && (pk.sign_type == signType::S_EQUAL))
				{
					pos_ += 1;
					return { true, binaryOperation::LESS_OR_EQUAL };
				}
				return { true, binaryOperation::LESS_THAN };
			}
			if (cur.sign_type == signType::S_GREATER_THAN)
			{
				if (pk.isValid() && (pk.sign_type == signType::S_EQUAL))
				{
					pos_ += 1;
					return { true, binaryOperation::GREATER_OR_EQUAL };
				}
				return { true, binaryOperation::GREATER_THAN };
			}
			break;
		case 4:	// == !=
			if (pk.isValid() && (pk.sign_type == signType::S_EQUAL))
			{
				if (cur.sign_type == signType::S_EQUAL)
				{
					pos_ += 1;
					return { true,binaryOperation::EQUAL };
				}
				if (cur.sign_type == signType::P_EXCL_MARK)
				{
					pos_ += 1;
					return { true,binaryOperation::NOTEQUAL };
				}
			}
			break;
		case 5:
			if (pk.isValid() && (pk.sign_type == signType::S_AND) && (cur.sign_type == signType::S_AND))
			{
				pos_ += 1;
				return { true,binaryOperation::AND };
			}
			break;
		case 6:
			if (pk.isValid() && (pk.sign_type == signType::S_VBAR) && (cur.sign_type == signType::S_VBAR))
			{
				pos_ += 1;
				return { true,binaryOperation::OR };
			}
			break;
		case 7:
			if ((cur.sign_type == signType::S_EQUAL)
				&&
				!(pk.isValid() && (pk.sign_type == signType::S_EQUAL))) // Refuse if op is ==
				return { true,binaryOperation::ASSIGN };
			break;
		default:
			throw Exceptions::parser_critical_error("Requested to match a Binary Operator with a non-existent priority");
			break;
	}
	pos_ -= 1;	// We did not find anything, decrement & return.
	return { false, binaryOperation::PASS };
}
