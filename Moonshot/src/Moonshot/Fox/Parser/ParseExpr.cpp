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
		first = parseCastExpr();	// We are at 0 ? Parse the castExpr then 

	if (!first)					// Check if it was found/parsed correctly. If not, return a null node, because this isn't an expression.
		return nullptr;

	rtr->setChild(dir::LEFT,first);	// Make first the left child of the return node !
	while (true)
	{
		// Match binary operator
		binaryOperation op;
		bool matchResult;
		std::tie(matchResult, op) = matchBinaryOp(priority);
		if (!matchResult) // No operator found : break.
			break;
		
		// Create a tmp node "second" that holds the RHS of the expression
		std::unique_ptr<IASTExpr> second;
		if (priority > 0) second = parseExpr(priority - 1);
		else second = parseCastExpr();

		if (!second) // Check for validity : we need a rhs. if we don't have one, we have an error !
		{
			errorExpected("Expected an expression after binary operator.");
			break;
		}

		// Add the node to the tree but in different ways, depending on left or right assoc.
		if (!isRightAssoc(op))		// Left associative operator
		{
			if (rtr->op_ == binaryOperation::PASS)
				rtr->op_ = op;
			else	// Already has one ? create a new node and make rtr its left child with oneUpNode
				rtr = oneUpNode(rtr, op);
			rtr->setChild(dir::RIGHT,second); // Set second as the child of the new node
		}									
		else	// right associative nodes
		{			
			// First "loop" check -> set rtr's op.
			if (rtr->op_ == binaryOperation::PASS)
				rtr->op_ = op;			

			if (!tmp) // tmp is empty
				tmp = std::move(second); // Set tmp to second.
			else
			{
				auto new_binop_node = std::make_unique<ASTBinaryExpr>(op); 
				new_binop_node->setChild(dir::LEFT,tmp); // set tmp as the left child of the new node

				// Here I store new_binop_node into a IASTExpr unique ptr, so it can be passed as argument in "makeChildOfDeepestNode"
				std::unique_ptr<IASTExpr> newnode = std::move(new_binop_node); 
				tmp = std::move(second);// Set second as the new tmp
				// make child
				rtr->makeChildOfDeepestNode(dir::RIGHT, newnode);
			}
		}
	}
	if (tmp) // Last isn't empty -> make it the right child of our last node.
	{
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
	auto cur = getToken();
	auto pk = getToken(pos_ + 1);
	// Check current Token validity
	if (!cur.isValid() || (cur.type != tokenType::TT_SIGN))
		return { false, binaryOperation::PASS };
	pos_ += 1; // We already increment once here in prevision of a matched operator. We'll decrease before returning the result if nothing was found, of course.

	switch (priority)
	{
		case 0: // **
			if (cur.sign_type == signType::S_ASTERISK)
			{
				if (pk.isValid() && pk.sign_type == signType::S_ASTERISK)
				{
					pos_++;
					return { true, binaryOperation::EXP };
				}
			}
			break;
		case 1: // * / %
			if (cur.sign_type == signType::S_ASTERISK)
			{
				if (pk.sign_type != signType::S_ASTERISK) // Disambiguation between '**' and '*'
					return { true, binaryOperation::MUL };
			}
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
