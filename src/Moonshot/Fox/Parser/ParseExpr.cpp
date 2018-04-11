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

#include "Moonshot/Fox/Basic/Exceptions.hpp"
#include "Moonshot/Fox/AST/ASTExpr.hpp"
#include <cassert>

using namespace Moonshot;

ParsingResult<IASTDeclRef*> Parser::parseArrayAccess(std::unique_ptr<IASTDeclRef> base)
{
	// 	<array_access>	= '[' <expr> ']'
	if (matchSign(SignType::S_SQ_OPEN))
	{
		if (auto expr = parseExpr())
		{
			if (matchSign(SignType::S_SQ_CLOSE))
				return ParsingResult<IASTDeclRef*>(ParsingOutcome::SUCCESS, std::make_unique<ASTArrayAccess>(std::move(base), std::move(expr.result_)));
			else
			{
				errorExpected("Expected a ']'");
				if (resyncToSign(SignType::S_SQ_CLOSE))
					return ParsingResult<IASTDeclRef*>(ParsingOutcome::FAILED_BUT_RECOVERED, std::move(base));
				return ParsingResult<IASTDeclRef*>(ParsingOutcome::FAILED_AND_DIED);
			}
		}
		else
		{
			errorExpected("Expected an expression");
			if(matchSign(SignType::S_SQ_CLOSE))
				// If failure, return the base so it isn't lost
				return ParsingResult<IASTDeclRef*>(ParsingOutcome::FAILED_BUT_RECOVERED,std::move(base));
			else
			{
				errorExpected("Expected a ']'");
				if(resyncToSign(SignType::S_SQ_CLOSE))
					return ParsingResult<IASTDeclRef*>(ParsingOutcome::FAILED_BUT_RECOVERED, std::move(base));
				return ParsingResult<IASTDeclRef*>(ParsingOutcome::FAILED_AND_DIED);
			}
		}
	}
	return ParsingResult<IASTDeclRef*>(ParsingOutcome::NOTFOUND,std::move(base));
}

ParsingResult<IASTDeclRef*> Parser::parseDeclCall()
{
	if (auto id = matchID())
	{
		std::unique_ptr<IASTDeclRef> expr = nullptr;
		if (auto exprlist = parseParensExprList())
		{
			// if an expression list is found create a functioncall node and set expr to that node.
			auto fcall = std::make_unique<ASTFunctionCallExpr>();
			fcall->setFunctionName(id.result_);
			fcall->setExprList(std::move(exprlist.result_));
			expr = std::move(fcall);
		}
		else // if no expression list found, make expr a declref to the identifier.
			expr = std::make_unique<ASTDeclRefExpr>(id.result_);

		assert(expr && "Expr is not set?");

		auto arrAcc = parseArrayAccess(std::move(expr));
		while(arrAcc.getFlag() == ParsingOutcome::SUCCESS)
		{
			assert(arrAcc.result_ && "parseArrayAccess returned a null node on success?");
			// Keep parsing with the latest result.
			arrAcc = parseArrayAccess(std::move(arrAcc.result_));
		}
		
		// Parsing array access done, extract the node.
		if ((arrAcc.getFlag() == ParsingOutcome::NOTFOUND) ||
			(arrAcc.getFlag() == ParsingOutcome::FAILED_BUT_RECOVERED))
		{
			assert(arrAcc.result_ && "parseArrayAccess did not return the base on failure?");
			expr = std::move(arrAcc.result_);
		}
		else if (arrAcc.getFlag() == ParsingOutcome::FAILED_AND_DIED)
			return ParsingResult<IASTDeclRef*>(ParsingOutcome::FAILED_AND_DIED);
		else
			throw std::exception("Unexpected  return type for parseArrayAccess");
		
		return ParsingResult<IASTDeclRef*>(ParsingOutcome::SUCCESS, std::move(expr));
	}
	return ParsingResult<IASTDeclRef*>(ParsingOutcome::NOTFOUND);
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
	if (auto match_lit = parseLiteral())
		return match_lit;
	// = <decl_call>
	else if (auto match_decl = parseDeclCall())
		return ParsingResult<IASTExpr*>(match_decl.getFlag(),std::move(match_decl.result_));
	// = '(' <expr> ')'
	else if (auto res = parseParensExpr())
		return res;
	return ParsingResult<IASTExpr*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTExpr*> Parser::parseMemberAccess()
{
	// <member_access>	= <primary> { '.' <decl_call> }
	if (auto prim = parsePrimary())
	{
		std::unique_ptr<IASTExpr> cur = std::move(prim.result_);
		while (matchSign(SignType::S_DOT))
		{
			if (auto declcall = parseDeclCall())
				cur = std::make_unique<ASTMemberAccessExpr>(std::move(cur), std::move(declcall.result_));
			else
			{
				errorExpected("Expected an identifier");
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
			return ParsingResult<IASTExpr*>(
				ParsingOutcome::SUCCESS,
				std::make_unique<ASTBinaryExpr>(
						binaryOperator::EXP,
						std::move(val.result_),
						std::move(prefix_expr.result_)
					)
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
			return ParsingResult<IASTExpr*>(
				ParsingOutcome::SUCCESS,
				std::make_unique<ASTUnaryExpr>(matchUop.result_, std::move(parseres.result_))
			);
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
		// Search for a (optional) cast: "as" <type>
		if (matchKeyword(KeywordType::KW_AS))
		{
			if (auto castType = matchTypeKw())
			{
				// If found, apply it to current node.
				return ParsingResult<IASTExpr*>(
					ParsingOutcome::SUCCESS,
					std::make_unique<ASTCastExpr>(castType.result_, std::move(parse_res.result_))
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

	ParsingOutcome outcome = ParsingOutcome::SUCCESS;

	rtr->setLHS(std::move(first));	// Make first the left child of the return node !
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
			outcome = ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY;
			break; 
		}

		if (rtr->getOp() == binaryOperator::DEFAULT) // if the node has still a "pass" operation
				rtr->setOp(matchResult.result_);
		else // if the node already has an operation
			rtr = oneUpNode(std::move(rtr), matchResult.result_);

		rtr->setRHS(std::move(second)); // Set second as the child of the node.
	}

	// When we have simple node (DEFAULT operation with only a value/expr as left child), we simplify it(only return the left child)
	auto simple = rtr->getSimple();
	if (simple)
		return ParsingResult<IASTExpr*>(outcome,std::move(simple));
	return ParsingResult<IASTExpr*>(outcome, std::move(rtr));
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

			return ParsingResult<IASTExpr*>(
					ParsingOutcome::SUCCESS,
					std::make_unique<ASTBinaryExpr>(
							matchResult.result_,
							std::move(lhs_res.result_),
							std::move(rhs_res.result_)
						)
				);
		}
		return ParsingResult<IASTExpr*>(ParsingOutcome::SUCCESS, std::move(lhs_res.result_));
	}
	else 
		return ParsingResult<IASTExpr*>(lhs_res.getFlag());
}

ParsingResult<IASTExpr*> Parser::parseParensExpr(const bool& isMandatory, const bool& isExprMandatory)
{
	if (matchSign(SignType::S_ROUND_OPEN))
	{
		ParsingOutcome ps = ParsingOutcome::SUCCESS;
		std::unique_ptr<IASTExpr> rtr;
		if (auto parseres = parseExpr())
			rtr = std::move(parseres.result_);
		else if (isExprMandatory)
		{
			errorExpected("Expected an expression");
			ps = ParsingOutcome::FAILED_BUT_RECOVERED;
		}

		if (!matchSign(SignType::S_ROUND_CLOSE))
		{
			errorExpected("Expected a ')' ,");
			if (!resyncToSign(SignType::S_ROUND_CLOSE))
				return ParsingResult<IASTExpr*>(ParsingOutcome::FAILED_AND_DIED);
			return ParsingResult<IASTExpr*>(ParsingOutcome::FAILED_BUT_RECOVERED, std::move(rtr));
		}

		return ParsingResult<IASTExpr*>(ps, std::move(rtr));
	}
	// failure
	if (isMandatory)
	{
		// attempt resync to )
		errorExpected("Expected a '('");
		if(resyncToSign(SignType::S_ROUND_CLOSE))
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
		while (auto comma = matchSign(SignType::S_COMMA))
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
	if (auto op_rb = matchSign(SignType::S_ROUND_OPEN))
	{
		auto exprlist = std::make_unique<ExprList>();
		if (auto parsedlist = parseExprList())			// optional expr_list
			exprlist = std::move(parsedlist.result_);

		// mandatory ')'
		if (!matchSign(SignType::S_ROUND_CLOSE))
		{
			// attempt resync to )
			errorExpected("Expected a ')'");
			if (resyncToSign(SignType::S_ROUND_CLOSE))
				return ParsingResult<ExprList*>(ParsingOutcome::FAILED_BUT_RECOVERED);
			return ParsingResult<ExprList*>(ParsingOutcome::FAILED_AND_DIED);
		}
		return ParsingResult<ExprList*>(ParsingOutcome::SUCCESS, std::move(exprlist));
	}
	return ParsingResult<ExprList*>(ParsingOutcome::NOTFOUND);
}

std::unique_ptr<ASTBinaryExpr> Parser::oneUpNode(std::unique_ptr<ASTBinaryExpr> node, const binaryOperator & op)
{
	auto newnode = std::make_unique<ASTBinaryExpr>(op,std::move(node));
	return newnode;
}

bool Parser::matchExponentOp()
{
	auto backup = createParserStateBackup();
	if (matchSign(SignType::S_ASTERISK))
	{
		if (matchSign(SignType::S_ASTERISK))
			return true;
		restoreParserStateFromBackup(backup);
	}
	return false;
}

ParsingResult<binaryOperator> Parser::matchAssignOp()
{
	auto backup = createParserStateBackup();
	if (matchSign(SignType::S_EQUAL))
	{
		// Try to match a S_EQUAL. If failed, that means that the next token isn't a =
		// If it succeeds, we founda '==', this is the comparison operator and we must backtrack to prevent errors.
		if (!matchSign(SignType::S_EQUAL))
			return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::ASSIGN_BASIC);
		restoreParserStateFromBackup(backup);
	}
	return ParsingResult<binaryOperator>(ParsingOutcome::NOTFOUND);
}

ParsingResult<unaryOperator> Parser::matchUnaryOp()
{
	if (matchSign(SignType::S_EXCL_MARK))
		return ParsingResult<unaryOperator>(ParsingOutcome::SUCCESS, unaryOperator::LOGICNOT);
	else if (matchSign(SignType::S_MINUS))
		return ParsingResult<unaryOperator>(ParsingOutcome::SUCCESS, unaryOperator::NEGATIVE);
	else if (matchSign(SignType::S_PLUS))
		return ParsingResult<unaryOperator>(ParsingOutcome::SUCCESS, unaryOperator::POSITIVE);
	return ParsingResult<unaryOperator>(ParsingOutcome::NOTFOUND);
}

ParsingResult<binaryOperator> Parser::matchBinaryOp(const char & priority)
{
	auto backup = createParserStateBackup();

	// Check current Token validity, also check if it's a sign because if it isn't we can return directly!
	if (!getToken().isValid() || !getToken().isSign())
		return ParsingResult<binaryOperator>(ParsingOutcome::NOTFOUND);

	switch (priority)
	{
		case 0: // * / %
			if (matchSign(SignType::S_ASTERISK))
			{
				if (!peekSign(getCurrentPosition(),SignType::S_ASTERISK)) // Disambiguation between '**' and '*'
					return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::MUL );
				restoreParserStateFromBackup(backup);; // Backtrack if we didn't return.
			}
			if (matchSign(SignType::S_SLASH))
				return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::DIV);
			if (matchSign(SignType::S_PERCENT))
				return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::MOD);
			break;
		case 1: // + -
			if (matchSign(SignType::S_PLUS))
				return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::ADD );
			if (matchSign(SignType::S_MINUS))
				return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::MINUS);
			break;
		case 2: // > >= < <=
			if (matchSign(SignType::S_LESS_THAN))
			{
				if (matchSign(SignType::S_EQUAL))
					return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::LESS_OR_EQUAL );
				return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::LESS_THAN );
			}
			if (matchSign(SignType::S_GREATER_THAN))
			{
				if (matchSign(SignType::S_EQUAL))
					return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::GREATER_OR_EQUAL );
				return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::GREATER_THAN );
			}
			break;
		case 3:	// == !=
			// try to match '=' twice.
			if (matchSign(SignType::S_EQUAL))
			{
				if (matchSign(SignType::S_EQUAL))
					return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::EQUAL );
				restoreParserStateFromBackup(backup);; // Backtrack if we didn't return.
			}
			if (matchSign(SignType::S_EXCL_MARK))
			{
				if (matchSign(SignType::S_EQUAL))
					return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::NOTEQUAL);
				restoreParserStateFromBackup(backup);; // Backtrack if we didn't return.
			}
			break;
		case 4: // &&
			if (matchSign(SignType::S_AMPERSAND))
			{
				if (matchSign(SignType::S_AMPERSAND))
					return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::LOGIC_AND);
				restoreParserStateFromBackup(backup);; // Backtrack if we didn't return.
			}
			break;
		case 5: // ||
			if (matchSign(SignType::S_VBAR))
			{
				if (matchSign(SignType::S_VBAR))
					return ParsingResult<binaryOperator>(ParsingOutcome::SUCCESS, binaryOperator::LOGIC_OR);
				restoreParserStateFromBackup(backup);; // Backtrack if we didn't return.
			}
			break;
		default:
			throw Exceptions::parser_critical_error("Requested to match a Binary Operator of unknown priority");
			break;
	}
	return ParsingResult<binaryOperator>(ParsingOutcome::NOTFOUND);
}
