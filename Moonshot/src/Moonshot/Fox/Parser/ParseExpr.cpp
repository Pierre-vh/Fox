////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ParseExpr.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
// This file implements expressions related methods (rules)	
////------------------------------------------------------////

#include "Parser.hpp"

using namespace Moonshot;
using namespace fv_util;

// Context and Exceptions
#include "Moonshot/Common/Context/Context.hpp"
#include "Moonshot/Common/Exceptions/Exceptions.hpp"

std::unique_ptr<IASTExpr> Parser::parseCallable()
{
	// = <id>
	auto result = matchID();
	if (result.first)
		return std::make_unique<ASTVarCall>(result.second);
	return nullptr;
}

std::unique_ptr<IASTExpr> Parser::parseValue()
{
	// if the Token is invalid, return directly a null node

	// = <const>
	auto matchValue_result = matchLiteral();
	if (matchValue_result.first) // if we have a value, return it packed in a ASTLiteral
		return std::make_unique<ASTLiteral>(matchValue_result.second.lit_val);
	else if (auto node = parseCallable()) // Callable?
		return node;
	// = '(' <expr> ')'
	else if (matchSign(sign::B_ROUND_OPEN))
	{
		auto expr = parseExpr(); // Parse the expression inside
		if (!expr) // check validity of the parsed expression
		{
			errorExpected("Expected an expression after '('.");
			return nullptr;
		}
		// retrieve the closing bracket, throw an error if we don't have one. 
		if (!matchSign(sign::B_ROUND_CLOSE))
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

std::unique_ptr<IASTExpr> Parser::parseExponentExpr()
{
	if (auto val = parseValue())
	{
		if (matchExponentOp())
		{
			auto prefix_expr = parsePrefixExpr();
			if (!prefix_expr)
			{
				errorExpected("Expected an expression after exponent operator.");
				return nullptr;
			}
			std::unique_ptr<ASTBinaryExpr> bin = std::make_unique<ASTBinaryExpr>();
			bin->op_ = binaryOperation::EXP;
			bin->left_ = std::move(val);
			bin->right_ = std::move(prefix_expr);
			return bin;
		}
		return val;
	}
	return nullptr;
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
	else if (auto node = parseExponentExpr())
		return node;
	return nullptr;
}

std::unique_ptr<IASTExpr> Parser::parseCastExpr()
{
	if (auto node = parsePrefixExpr())
	{
		std::size_t casttype = indexes::invalid_index;
		// Search for a (optional) cast: "as" <type>
		if (matchKeyword(keyword::TC_AS))
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

std::unique_ptr<IASTExpr> Parser::parseBinaryExpr(const char & priority)
{
	auto rtr = std::make_unique<ASTBinaryExpr>(binaryOperation::PASS);

	// change this to ASTBinaryExpr
	std::unique_ptr<IASTExpr> first = 0;

	if (priority > 0) first = parseBinaryExpr(priority - 1);	// Go down in precedence
	else first = parseCastExpr();	// We are at 0 ? Parse the castExpr then 

	if (!first)					
		return nullptr;

	rtr->setChild(dir::LEFT, first);	// Make first the left child of the return node !
	while (true)
	{
		// Match binary operator
		binaryOperation op;
		bool matchResult;
		std::tie(matchResult, op) = matchBinaryOp(priority);
		if (!matchResult) // No operator found : break.
			break;

		// Create a node "second" that holds the RHS of the expression
		std::unique_ptr<IASTExpr> second;
		if (priority > 0) second = parseBinaryExpr(priority - 1);
		else second = parseCastExpr();

		if (!second) // Check for validity : we need a rhs. if we don't have one, we have an error !
		{
			errorExpected("Expected an expression after binary operator.");
			break;
		}

		if (rtr->op_ == binaryOperation::PASS) // if the node has still a "pass" operation
				rtr->op_ = op;
		else // if the node already has an operation
			rtr = oneUpNode(rtr, op);

		rtr->setChild(dir::RIGHT, second); // Set second as the child of the node.
	}

	// When we have simple node (PASS operation with only a value/expr as left child), we simplify it(only return the left child)
	auto simple = rtr->getSimple();
	if (simple)
		return simple;
	return rtr;
}

std::unique_ptr<IASTExpr> Parser::parseExpr()
{
	if (auto lhs = parseBinaryExpr())
	{
		auto matchResult = matchAssignOp();
		if (matchResult.first)
		{
			auto rhs = parseExpr();
			if (!rhs)
			{
				errorExpected("Expected expression after assignement operator.");
				return nullptr;
			}

			std::unique_ptr<ASTBinaryExpr> binexpr = std::make_unique<ASTBinaryExpr>();
		
			binexpr->op_ = matchResult.second;
			binexpr->left_ = std::move(lhs);
			binexpr->right_ = std::move(rhs);

			return binexpr;
		}
		return lhs;
	}
	return nullptr;
}

std::unique_ptr<ASTBinaryExpr> Parser::oneUpNode(std::unique_ptr<ASTBinaryExpr>& node, const binaryOperation & op)
{
	auto newnode = std::make_unique<ASTBinaryExpr>(op);
	newnode->left_ = std::move(node);
	return newnode;
}

bool Moonshot::Parser::matchExponentOp()
{
	auto cur = getToken();
	auto pk = getToken(pos_ + 1);
	if (cur.isValid() && cur.type == tokenCat::TT_SIGN && cur.sign_type == sign::S_ASTERISK)
	{
		if (pk.isValid() && pk.type == tokenCat::TT_SIGN && pk.sign_type == sign::S_ASTERISK)
		{
			pos_+=2;
			return true;
		}
	}
	return false;
}

std::pair<bool, binaryOperation> Parser::matchAssignOp()
{
	auto cur = getToken();
	pos_++;
	if (cur.isValid() && cur.type == tokenCat::TT_SIGN && cur.sign_type == sign::S_EQUAL)
		return { true,binaryOperation::ASSIGN };
	pos_--;
	return { false,binaryOperation::PASS };
}

std::pair<bool, unaryOperation> Parser::matchUnaryOp()
{
	auto cur = getToken();
	if (!cur.isValid() || (cur.type != tokenCat::TT_SIGN))
		return { false, unaryOperation::DEFAULT };
	pos_++;

	if (cur.sign_type == sign::P_EXCL_MARK)
		return { true, unaryOperation::LOGICNOT };

	if (cur.sign_type == sign::S_MINUS)
		return { true, unaryOperation::NEGATIVE};

	if (cur.sign_type == sign::S_PLUS)
		return { true, unaryOperation::POSITIVE};

	pos_--;
	return { false, unaryOperation::DEFAULT };
}

std::pair<bool, binaryOperation> Parser::matchBinaryOp(const char & priority)
{
	auto cur = getToken();
	auto pk = getToken(pos_ + 1);
	// Check current Token validity
	if (!cur.isValid() || (cur.type != tokenCat::TT_SIGN))
		return { false, binaryOperation::PASS };
	pos_ += 1; // We already increment once here in prevision of a matched operator. We'll decrease before returning the result if nothing was found, of course.

	switch (priority)
	{
		case 0: // * / %
			if (cur.sign_type == sign::S_ASTERISK)
			{
				if (pk.sign_type != sign::S_ASTERISK) // Disambiguation between '**' and '*'
					return { true, binaryOperation::MUL };
			}
			if (cur.sign_type == sign::S_SLASH)
				return { true, binaryOperation::DIV };
			if (cur.sign_type == sign::S_PERCENT)
				return { true, binaryOperation::MOD };
			break;
		case 1: // + -
			if (cur.sign_type == sign::S_PLUS)
				return { true, binaryOperation::ADD };
			if (cur.sign_type == sign::S_MINUS)
				return { true, binaryOperation::MINUS };
			break;
		case 2: // > >= < <=
			if (cur.sign_type == sign::S_LESS_THAN)
			{
				if (pk.isValid() && (pk.sign_type == sign::S_EQUAL))
				{
					pos_ += 1;
					return { true, binaryOperation::LESS_OR_EQUAL };
				}
				return { true, binaryOperation::LESS_THAN };
			}
			if (cur.sign_type == sign::S_GREATER_THAN)
			{
				if (pk.isValid() && (pk.sign_type == sign::S_EQUAL))
				{
					pos_ += 1;
					return { true, binaryOperation::GREATER_OR_EQUAL };
				}
				return { true, binaryOperation::GREATER_THAN };
			}
			break;
		case 3:	// == !=
			if (pk.isValid() && (pk.sign_type == sign::S_EQUAL))
			{
				if (cur.sign_type == sign::S_EQUAL)
				{
					pos_ += 1;
					return { true,binaryOperation::EQUAL };
				}
				if (cur.sign_type == sign::P_EXCL_MARK)
				{
					pos_ += 1;
					return { true,binaryOperation::NOTEQUAL };
				}
			}
			break;
		case 4:
			if (pk.isValid() && (pk.sign_type == sign::S_AND) && (cur.sign_type == sign::S_AND))
			{
				pos_ += 1;
				return { true,binaryOperation::AND };
			}
			break;
		case 5:
			if (pk.isValid() && (pk.sign_type == sign::S_VBAR) && (cur.sign_type == sign::S_VBAR))
			{
				pos_ += 1;
				return { true,binaryOperation::OR };
			}
			break;
		default:
			throw Exceptions::parser_critical_error("Requested to match a Binary Operator with a non-existent priority");
			break;
	}
	pos_ -= 1;	// We did not find anything, decrement & return.
	return { false, binaryOperation::PASS };
}
