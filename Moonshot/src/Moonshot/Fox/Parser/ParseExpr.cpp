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
// Context and Exceptions
#include "Moonshot/Common/Context/Context.hpp"
#include "Moonshot/Common/Exceptions/Exceptions.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTExpr.hpp"
using namespace Moonshot;

using category = Token::category;
using sign = Token::sign;
using keyword = Token::keyword;



ParsingResult<IASTExpr*> Parser::parseDeclCall()
{
	if (auto id = matchID())
	{
		auto declref = std::make_unique<ASTDeclRefExpr>();
		declref->setDeclnameStr(id.result_);
		if (auto exprlist = parseParensExprList())
		{
			auto fcall = std::make_unique<ASTFunctionCallExpr>();
			fcall->setDeclRef(std::move(declref));
			fcall->setExprList(std::move(exprlist.result_));
			return ParsingResult<IASTExpr*>(ParsingOutcome::SUCCESS,std::move(fcall));
		}
		return ParsingResult<IASTExpr*>(ParsingOutcome::SUCCESS, std::move(declref));
	}
	return ParsingResult<IASTExpr*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTExpr*> Parser::parseLiteral()
{
	if (auto matchres = matchLiteral())
		return ParsingResult<IASTExpr*>(ParsingOutcome::SUCCESS, std::make_unique<ASTLiteralExpr>(matchres.result_));
	return ParsingResult<IASTExpr*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTExpr*>  Parser::parsePrimary()
{
	// = <literal>
	if (auto match_lit = parseLiteral()) // if we have a literal, return it packed in a ASTLiteralExpr
		return match_lit;
	// = <decl_call>
	else if (auto match_decl = parseDeclCall())
		return match_decl;
	// = '(' <expr> ')'
	else if (auto res = parseParensExpr())
		return res;
	return ParsingResult<IASTExpr*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTExpr*> Parser::parseMemberAccess()
{
	// <member_access>	= <primary> { '.' <id> [ <parens_expr_list> ] }
	if (auto prim = parsePrimary())
	{
		std::unique_ptr<IASTExpr> cur = std::move(prim.result_);

		while (auto dot = matchSign(sign::P_DOT))
		{
			if (auto id = matchID())
			{
				auto mem_access_node = std::make_unique<ASTMemberOfExpr>();
				mem_access_node->setBase(std::move(cur));
				mem_access_node->setDeclname(id.result_);

				if (auto fn_arg = parseParensExprList())
				{
					auto fn_call_node = std::make_unique<ASTFunctionCallExpr>();
					fn_call_node->setDeclRef(std::move(mem_access_node));
					fn_call_node->setExprList(std::move(fn_arg.result_));
					cur = std::move(fn_call_node);
				}
				else if (fn_arg.getFlag() == ParsingOutcome::NOTFOUND)	
					cur = std::move(mem_access_node); // else just move the member access node.
				else
					return ParsingResult<IASTExpr*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
			}
			else
			{
				errorExpected("Expected identifier");
				return ParsingResult<IASTExpr*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
			}
		}
		return ParsingResult<IASTExpr*>(ParsingOutcome::SUCCESS, std::move(cur));
	}
	return ParsingResult<IASTExpr*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTExpr*> Parser::parseExponentExpr()
{
	if (auto val = parseMemberAccess())
	{
		if (matchExponentOp())
		{
			auto prefix_expr = parsePrefixExpr();
			if (!prefix_expr)
			{
				errorExpected("Expected an expression after exponent operator.");
				return ParsingResult<IASTExpr*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
			}
			std::unique_ptr<ASTBinaryExpr> bin = std::make_unique<ASTBinaryExpr>();
			bin->op_ = binaryOperator::EXP;
			bin->left_ = std::move(val.result_);
			bin->right_ = std::move(prefix_expr.result_);
			return ParsingResult<IASTExpr*>(
				ParsingOutcome::SUCCESS,
				std::move(bin)
			);
		}
		return val;
	}
	return ParsingResult<IASTExpr*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTExpr*> Parser::parsePrefixExpr()
{
	if (auto matchUop = matchUnaryOp())
	{
		if (auto parseres = parsePrefixExpr())
		{
			auto rtr = std::make_unique<ASTUnaryExpr>();
			rtr->op_ = matchUop.result_;
			rtr->child_ = std::move(parseres.result_);
			return ParsingResult<IASTExpr*>(ParsingOutcome::SUCCESS,std::move(rtr));
		}
		else
		{
			errorExpected("Expected an expression after unary operator in prefix expression.");
			return ParsingResult<IASTExpr*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
	}
	else if (auto expExpr = parseExponentExpr())
		return expExpr;
	return ParsingResult<IASTExpr*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTExpr*>  Parser::parseCastExpr()
{
	if (auto parse_res = parsePrefixExpr())
	{
		std::size_t casttype = TypeIndex::InvalidIndex;
		// Search for a (optional) cast: "as" <type>
		if (matchKeyword(keyword::TC_AS))
		{
			if (auto castType = matchTypeKw())
			{
				// If found, apply it to current node.
				auto rtr = std::make_unique<ASTCastExpr>();
				rtr->setCastGoal(castType.result_);
				rtr->child_ = std::move(parse_res.result_);
				return ParsingResult<IASTExpr*>(
						ParsingOutcome::SUCCESS,
						std::move(rtr)
				);
			}
			else
			{
				// If error (invalid keyword found, etc.)
				errorExpected("Expected a type keyword after \"as\" in cast expression.");
				return ParsingResult<IASTExpr*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
			}
		}
		return ParsingResult<IASTExpr*>(
				ParsingOutcome::SUCCESS,
				std::move(parse_res.result_)
		);
	}
	return ParsingResult<IASTExpr*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTExpr*>  Parser::parseBinaryExpr(const char & priority)
{
	auto rtr = std::make_unique<ASTBinaryExpr>(binaryOperator::DEFAULT);

	std::unique_ptr<IASTExpr> first;
	if (priority > 0)
	{
		auto res = parseBinaryExpr(priority - 1);
		if (res)
			first = std::move(res.result_);
	}
	else
	{
		auto res = parseCastExpr();
		if (res)
			first = std::move(res.result_);
	}

	if (!first)					
		return ParsingResult<IASTExpr*>(ParsingOutcome::NOTFOUND);

	rtr->left_ = std::move(first);	// Make first the left child of the return node !
	while (true)
	{
		// Match binary operator
		auto matchResult = matchBinaryOp(priority);
		if (!matchResult) // No operator found : break.
			break;

		// Create a node "second" that holds the RHS of the expression
		std::unique_ptr<IASTExpr> second;
		if (priority > 0)
		{
			auto res = parseBinaryExpr(priority - 1);
			if (res)
				second = std::move(res.result_);
		}
		else
		{
			auto res = parseCastExpr();
			if (res)
				second = std::move(res.result_);
		}

		if (!second) // Check for validity : we need a rhs. if we don't have one, we have an error !
		{
			errorExpected("Expected an expression after binary operator,");
			break; // We break instead of returning, so we can return the expressions that were parsed correctly so we don't cause an error cascade for the user.
		}

		if (rtr->op_ == binaryOperator::DEFAULT) // if the node has still a "pass" operation
				rtr->op_ = matchResult.result_;
		else // if the node already has an operation
			rtr = oneUpNode(std::move(rtr), matchResult.result_);

		rtr->right_ = std::move(second); // Set second as the child of the node.
	}

	// When we have simple node (DEFAULT operation with only a value/expr as left child), we simplify it(only return the left child)
	auto simple = rtr->getSimple();
	if (simple)
		return ParsingResult<IASTExpr*>(ParsingOutcome::SUCCESS,std::move(simple));
	return ParsingResult<IASTExpr*>(ParsingOutcome::SUCCESS, std::move(rtr));
}

ParsingResult<IASTExpr*> Parser::parseExpr()
{
	if (auto lhs_res = parseBinaryExpr())
	{
		auto matchResult = matchAssignOp();
		if (matchResult)
		{
			auto rhs_res = parseExpr();
			if (!rhs_res)
			{
				errorExpected("Expected expression after assignement operator.");
				return ParsingResult<IASTExpr*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
			}

			std::unique_ptr<ASTBinaryExpr> binexpr = std::make_unique<ASTBinaryExpr>();
		
			binexpr->op_ = matchResult.result_;
			binexpr->left_ = std::move(lhs_res.result_);
			binexpr->right_ = std::move(rhs_res.result_);

			return ParsingResult<IASTExpr*>(ParsingOutcome::SUCCESS,std::move(binexpr));
		}
		return ParsingResult<IASTExpr*>(ParsingOutcome::SUCCESS, std::move(lhs_res.result_));
	}
	return ParsingResult<IASTExpr*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTExpr*> Parser::parseParensExpr(const bool& isMandatory)
{
	if (matchSign(sign::B_ROUND_OPEN))
	{
		std::unique_ptr<IASTExpr> rtr;
		if (auto parseres = parseExpr())
			rtr = std::move(parseres.result_);

		if (!matchSign(sign::B_ROUND_CLOSE))
		{
			errorExpected("Expected a ')' after expression,");
			if (!resyncToDelimiter(sign::B_ROUND_CLOSE))
				return ParsingResult<IASTExpr*>(ParsingOutcome::FAILED_AND_DIED);
			return ParsingResult<IASTExpr*>(ParsingOutcome::FAILED_BUT_RECOVERED, std::move(rtr));
		}

		return ParsingResult<IASTExpr*>(ParsingOutcome::SUCCESS, std::move(rtr));
	}
	// failure
	if (isMandatory)
	{
		// attempt resync to )
		errorExpected("Expected a '('");
		if(resyncToDelimiter(sign::B_ROUND_CLOSE))
			return ParsingResult<IASTExpr*>(ParsingOutcome::FAILED_BUT_RECOVERED);
		return ParsingResult<IASTExpr*>(ParsingOutcome::FAILED_AND_DIED);
	}
	else 
		return ParsingResult<IASTExpr*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<ExprList*> Parser::parseExprList()
{
	// <expr_list> = <expr> {',' <expr> }
	if (auto firstexpr = parseExpr())
	{
		auto exprlist = std::make_unique<ExprList>();
		exprlist->addExpr(std::move(firstexpr.result_));
		while (auto comma = matchSign(sign::P_COMMA))
		{
			if (auto expr = parseExpr())
				exprlist->addExpr(std::move(expr.result_));
			else
			{
				errorExpected("Expected an expression");
				return ParsingResult<ExprList*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
			}
		}
		return ParsingResult<ExprList*>(ParsingOutcome::SUCCESS, std::move(exprlist));
	}
	return ParsingResult<ExprList*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<ExprList*> Parser::parseParensExprList()
{
	// <parens_expr_list>	= '(' [ <expr_list> ] ')'
	if (auto op_rb = matchSign(sign::B_ROUND_OPEN))
	{
		auto exprlist = std::make_unique<ExprList>();
		if (auto parsedlist = parseExprList())			// optional expr_list
			exprlist = std::move(parsedlist.result_);

		// mandatory ')'
		if (!matchSign(sign::B_ROUND_CLOSE))
		{
			// attempt resync to )
			errorExpected("Expected a ')'");
			if (resyncToDelimiter(sign::B_ROUND_CLOSE))
				return ParsingResult<ExprList*>(ParsingOutcome::FAILED_BUT_RECOVERED);
			return ParsingResult<ExprList*>(ParsingOutcome::FAILED_AND_DIED);
		}
		return ParsingResult<ExprList*>(ParsingOutcome::SUCCESS, std::move(exprlist));
	}
	return ParsingResult<ExprList*>(ParsingOutcome::NOTFOUND);
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

ParsingResult<binaryOperator> Parser::matchAssignOp()
{
	auto cur = getToken();
	state_.pos++;
	if (cur.isValid() && cur.type == category::SIGN && cur.sign_type == sign::S_EQUAL)
		return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::ASSIGN_BASIC);
	state_.pos--;
	return ParsingResult<binaryOperator>(ParsingOutcome::NOTFOUND);
}

ParsingResult<unaryOperator> Parser::matchUnaryOp()
{
	auto cur = getToken();
	state_.pos++;

	if (cur.sign_type == sign::P_EXCL_MARK)
		return ParsingResult<unaryOperator>(ParsingOutcome::SUCCESS, unaryOperator::LOGICNOT);

	if (cur.sign_type == sign::S_MINUS)
		return ParsingResult<unaryOperator>(ParsingOutcome::SUCCESS, unaryOperator::NEGATIVE);

	if (cur.sign_type == sign::S_PLUS)
		return ParsingResult<unaryOperator>(ParsingOutcome::SUCCESS, unaryOperator::POSITIVE);

	state_.pos--;
	return ParsingResult<unaryOperator>(ParsingOutcome::NOTFOUND);
}

ParsingResult<binaryOperator> Parser::matchBinaryOp(const char & priority)
{
	auto cur = getToken();
	auto pk = getToken(state_.pos + 1);
	// Check current Token validity
	if (!cur.isValid() || (cur.type != category::SIGN))
		return ParsingResult<binaryOperator>(ParsingOutcome::NOTFOUND);
	state_.pos += 1; // We already increment once here in prevision of a matched operator. We'll decrease before returning the result if nothing was found, of course.

	switch (priority)
	{
		case 0: // * / %
			if (cur.sign_type == sign::S_ASTERISK)
			{
				if (pk.sign_type != sign::S_ASTERISK) // Disambiguation between '**' and '*'
					return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::MUL );
			}
			if (cur.sign_type == sign::S_SLASH)
				return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::DIV);
			if (cur.sign_type == sign::S_PERCENT)
				return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::MOD);
			break;
		case 1: // + -
			if (cur.sign_type == sign::S_PLUS)
				return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::ADD );
			if (cur.sign_type == sign::S_MINUS)
				return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::MINUS);
			break;
		case 2: // > >= < <=
			if (cur.sign_type == sign::S_LESS_THAN)
			{
				if (pk.isValid() && (pk.sign_type == sign::S_EQUAL))
				{
					state_.pos += 1;
					return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::LESS_OR_EQUAL );
				}
				return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::LESS_THAN );
			}
			if (cur.sign_type == sign::S_GREATER_THAN)
			{
				if (pk.isValid() && (pk.sign_type == sign::S_EQUAL))
				{
					state_.pos += 1;
					return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::GREATER_OR_EQUAL );
				}
				return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::GREATER_THAN );
			}
			break;
		case 3:	// == !=
			if (pk.isValid() && (pk.sign_type == sign::S_EQUAL))
			{
				if (cur.sign_type == sign::S_EQUAL)
				{
					state_.pos += 1;
					return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::EQUAL );
				}
				if (cur.sign_type == sign::P_EXCL_MARK)
				{
					state_.pos += 1;
					return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::NOTEQUAL );
				}
			}
			break;
		case 4:
			if (pk.isValid() && (pk.sign_type == sign::S_AMPERSAND) && (cur.sign_type == sign::S_AMPERSAND))
			{
				state_.pos += 1;
				return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::LOGIC_AND);
			}
			break;
		case 5:
			if (pk.isValid() && (pk.sign_type == sign::S_VBAR) && (cur.sign_type == sign::S_VBAR))
			{
				state_.pos += 1;
				return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::LOGIC_OR);
			}
			break;
		default:
			throw Exceptions::parser_critical_error("Requested to match a Binary Operator of unknown priority");
			break;
	}
	state_.pos -= 1;	// We did not find anything, decrement & return.
	return ParsingResult<binaryOperator>(ParsingOutcome::NOTFOUND);
}
