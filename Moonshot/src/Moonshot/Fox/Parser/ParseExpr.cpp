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
using namespace TypeUtils;

using category = Token::category;
using sign = Token::sign;
using keyword = Token::keyword;

// Context and Exceptions
#include "Moonshot/Common/Context/Context.hpp"
#include "Moonshot/Common/Exceptions/Exceptions.hpp"

#include "Moonshot/Fox/AST/Nodes/ASTExpr.hpp"

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
			bin->op_ = binaryOperator::EXP;
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
	unaryOperator uopOp = unaryOperator::DEFAULT;
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
	auto rtr = std::make_unique<ASTBinaryExpr>(binaryOperator::PASS);

	// change this to ASTBinaryExpr
	std::unique_ptr<IASTExpr> first = 0;

	if (priority > 0) first = parseBinaryExpr(priority - 1);	// Go down in precedence
	else first = parseCastExpr();	// We are at 0 ? Parse the castExpr then 

	if (!first)					
		return nullptr;

	rtr->left_ = std::move(first);	// Make first the left child of the return node !
	while (true)
	{
		// Match binary operator
		binaryOperator op;
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
			errorExpected("Expected an expression after binary operator,");
			break;
		}

		if (rtr->op_ == binaryOperator::PASS) // if the node has still a "pass" operation
				rtr->op_ = op;
		else // if the node already has an operation
			rtr = oneUpNode(std::move(rtr), op);

		rtr->right_ = std::move(second); // Set second as the child of the node.
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

std::unique_ptr<ASTBinaryExpr> Parser::oneUpNode(std::unique_ptr<ASTBinaryExpr> node, const binaryOperator & op)
{
	auto newnode = std::make_unique<ASTBinaryExpr>(op);
	newnode->left_ = std::move(node);
	return newnode;
}

bool Parser::matchExponentOp()
{
	auto cur = getToken();
	auto pk = getToken(state_.pos + 1);
	if (cur.isValid() && cur.type == category::SIGN && cur.sign_type == sign::S_ASTERISK)
	{
		if (pk.isValid() && pk.type == category::SIGN && pk.sign_type == sign::S_ASTERISK)
		{
			state_.pos+=2;
			return true;
		}
	}
	return false;
}

std::pair<bool, binaryOperator> Parser::matchAssignOp()
{
	auto cur = getToken();
	state_.pos++;
	if (cur.isValid() && cur.type == category::SIGN && cur.sign_type == sign::S_EQUAL)
		return { true,binaryOperator::ASSIGN };
	state_.pos--;
	return { false,binaryOperator::PASS };
}

std::pair<bool, unaryOperator> Parser::matchUnaryOp()
{
	auto cur = getToken();
	if (!cur.isValid() || (cur.type != category::SIGN))
		return { false, unaryOperator::DEFAULT };
	state_.pos++;

	if (cur.sign_type == sign::P_EXCL_MARK)
		return { true, unaryOperator::LOGICNOT };

	if (cur.sign_type == sign::S_MINUS)
		return { true, unaryOperator::NEGATIVE};

	if (cur.sign_type == sign::S_PLUS)
		return { true, unaryOperator::POSITIVE};

	state_.pos--;
	return { false, unaryOperator::DEFAULT };
}

std::pair<bool, binaryOperator> Parser::matchBinaryOp(const char & priority)
{
	auto cur = getToken();
	auto pk = getToken(state_.pos + 1);
	// Check current Token validity
	if (!cur.isValid() || (cur.type != category::SIGN))
		return { false, binaryOperator::PASS };
	state_.pos += 1; // We already increment once here in prevision of a matched operator. We'll decrease before returning the result if nothing was found, of course.

	switch (priority)
	{
		case 0: // * / %
			if (cur.sign_type == sign::S_ASTERISK)
			{
				if (pk.sign_type != sign::S_ASTERISK) // Disambiguation between '**' and '*'
					return { true, binaryOperator::MUL };
			}
			if (cur.sign_type == sign::S_SLASH)
				return { true, binaryOperator::DIV };
			if (cur.sign_type == sign::S_PERCENT)
				return { true, binaryOperator::MOD };
			break;
		case 1: // + -
			if (cur.sign_type == sign::S_PLUS)
				return { true, binaryOperator::ADD };
			if (cur.sign_type == sign::S_MINUS)
				return { true, binaryOperator::MINUS };
			break;
		case 2: // > >= < <=
			if (cur.sign_type == sign::S_LESS_THAN)
			{
				if (pk.isValid() && (pk.sign_type == sign::S_EQUAL))
				{
					state_.pos += 1;
					return { true, binaryOperator::LESS_OR_EQUAL };
				}
				return { true, binaryOperator::LESS_THAN };
			}
			if (cur.sign_type == sign::S_GREATER_THAN)
			{
				if (pk.isValid() && (pk.sign_type == sign::S_EQUAL))
				{
					state_.pos += 1;
					return { true, binaryOperator::GREATER_OR_EQUAL };
				}
				return { true, binaryOperator::GREATER_THAN };
			}
			break;
		case 3:	// == !=
			if (pk.isValid() && (pk.sign_type == sign::S_EQUAL))
			{
				if (cur.sign_type == sign::S_EQUAL)
				{
					state_.pos += 1;
					return { true,binaryOperator::EQUAL };
				}
				if (cur.sign_type == sign::P_EXCL_MARK)
				{
					state_.pos += 1;
					return { true,binaryOperator::NOTEQUAL };
				}
			}
			break;
		case 4:
			if (pk.isValid() && (pk.sign_type == sign::S_AND) && (cur.sign_type == sign::S_AND))
			{
				state_.pos += 1;
				return { true,binaryOperator::AND };
			}
			break;
		case 5:
			if (pk.isValid() && (pk.sign_type == sign::S_VBAR) && (cur.sign_type == sign::S_VBAR))
			{
				state_.pos += 1;
				return { true,binaryOperator::OR };
			}
			break;
		default:
			throw Exceptions::parser_critical_error("Requested to match a Binary Operator with a non-existent priority");
			break;
	}
	state_.pos -= 1;	// We did not find anything, decrement & return.
	return { false, binaryOperator::PASS };
}
