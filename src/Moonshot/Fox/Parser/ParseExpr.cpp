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

// note, here the unique_ptr is passed by reference because it will be consumed (moved -> nulled) only if a '[' is found, else, it's left untouched.
ParsingResult<IASTDeclRef*> Parser::parseArrayAccess(std::unique_ptr<IASTDeclRef>& base)
{
	// 	<array_access>	= '[' <expr> ']'
	// '['
	if (matchSign(SignType::S_SQ_OPEN))
	{
		// <expr>
		if (auto expr = parseExpr())
		{
			// ']'
			if (!matchSign(SignType::S_SQ_CLOSE))
			{
				errorExpected("Expected a ']'");
				if (!resyncToSign(SignType::S_SQ_CLOSE))
					return ParsingResult<IASTDeclRef*>(false);
			}
			return ParsingResult<IASTDeclRef*>(std::make_unique<ASTArrayAccess>(std::move(base), std::move(expr.result)));
		}
		else
		{
			errorExpected("Expected an expression");
			resyncToSign(SignType::S_SQ_CLOSE); // try to resync, but we don't catch the result because we'll return false anyway.
			return ParsingResult<IASTDeclRef*>(false);
		}
	}
	return ParsingResult<IASTDeclRef*>();
}

ParsingResult<IASTDeclRef*> Parser::parseDeclCall()
{
	// <decl_call>		= <id> [ <parens_expr_list> ] { <array_access> }

	// <id>
	if (auto id = matchID())
	{
		// [ <parens_expr_list> ]
		std::unique_ptr<IASTDeclRef> expr = nullptr;
		if (auto exprlist = parseParensExprList())
		{
			// if an expression list is found create a functioncall node and set expr to that node.
			auto fcall = std::make_unique<ASTFunctionCallExpr>();
			fcall->setFunctionIdentifier(id);
			fcall->setExprList(std::move(exprlist.result));
			expr = std::move(fcall);
		}
		else // if no expression list found, make expr a declref to the identifier.
			expr = std::make_unique<ASTDeclRefExpr>(id);

		// { <array_access> }
		auto arrAcc = parseArrayAccess(expr);
		while(arrAcc)
		{
			expr = std::move(arrAcc.result);
			arrAcc = parseArrayAccess(expr);

			if (arrAcc.wasSuccessful() && !arrAcc.isUsable()) // "not found"
			{
				// when the parseArrayAccess method doesn't find anything, it should leave it's argument in place
				// and not move it! 
				// We're checking that this behaviour is respected with this assert.
				assert(expr && "parseArrayAccess consumed the expr without a success?");
			}
		}
		
		
		if(!arrAcc.wasSuccessful()) // parseArrayAccess ended on an error
			return ParsingResult<IASTDeclRef*>(false);
		else // ended on a "not found" -> normal behaviour, return.
			return ParsingResult<IASTDeclRef*>(std::move(expr));
	}
	return ParsingResult<IASTDeclRef*>();
}

ParsingResult<ASTExpr*> Parser::parseLiteral()
{
	if (auto matchres = matchLiteral())
	{
		auto litinfo = matchres.result;
		std::unique_ptr<ASTExpr> expr = nullptr;
		if (litinfo.isBool())
			expr = std::make_unique<ASTBoolLiteralExpr>(litinfo.get<bool>());
		else if (litinfo.isString())
			expr = std::make_unique<ASTStringLiteralExpr>(litinfo.get<std::string>());
		else if (litinfo.isChar())
			expr = std::make_unique<ASTCharLiteralExpr>(litinfo.get<CharType>());
		else if (litinfo.isInt())
			expr = std::make_unique<ASTIntegerLiteralExpr>(litinfo.get<IntType>());
		else if (litinfo.isFloat())
			expr = std::make_unique<ASTFloatLiteralExpr>(litinfo.get<FloatType>());
		else
			throw std::exception("Unknown literal type");

		return ParsingResult<ASTExpr*>(std::move(expr));
	}
	return ParsingResult<ASTExpr*>();
}

ParsingResult<ASTExpr*>  Parser::parsePrimary()
{
	// = <literal>
	if (auto match_lit = parseLiteral())
		return match_lit;
	else if(!match_lit.wasSuccessful())
		return ParsingResult<ASTExpr*>(false);

	// = <decl_call>
	if (auto match_decl = parseDeclCall())
		return ParsingResult<ASTExpr*>(std::move(match_decl.result));
	else if(!match_decl.wasSuccessful())
		return ParsingResult<ASTExpr*>(false);

	// = '(' <expr> ')'
	if (auto parens_expr = parseParensExpr())
		return parens_expr;
	else if (!parens_expr.wasSuccessful())
		return ParsingResult<ASTExpr*>(false);

	return ParsingResult<ASTExpr*>();
}

ParsingResult<ASTExpr*> Parser::parseMemberAccess()
{
	// <member_access>	= <primary> { '.' <decl_call> }
	if (auto prim = parsePrimary())
	{
		std::unique_ptr<ASTExpr> lhs = std::move(prim.result);
		// '.'
		while (matchSign(SignType::S_DOT))
		{
			// <decl_call>
			if (auto rhs = parseDeclCall())
				lhs = std::make_unique<ASTMemberAccessExpr>(std::move(lhs), std::move(rhs.result));
			else
			{
				errorExpected("Expected an identifier");
				return ParsingResult<ASTExpr*>(false);
			}
		}
		return ParsingResult<ASTExpr*>(std::move(lhs));
	}
	else
		return ParsingResult<ASTExpr*>(prim.wasSuccessful());
}

ParsingResult<ASTExpr*> Parser::parseExponentExpr()
{
	// <exp_expr>	= <member_access> [ <exponent_operator> <prefix_expr> ]
	// <member_access>
	if (auto rhs = parseMemberAccess())
	{
		// <exponent_operator> 
		if (matchExponentOp())
		{
			// <prefix_expr>
			auto lhs = parsePrefixExpr();
			if (!lhs)
			{
				if(lhs.wasSuccessful())
					errorExpected("Expected an expression after exponent operator.");
				return ParsingResult<ASTExpr*>(false);
			}
			return ParsingResult<ASTExpr*>(
				std::make_unique<ASTBinaryExpr>(
						binaryOperator::EXP,
						std::move(lhs.result),
						std::move(rhs.result)
					)
			);
		}
		// only <member_access>
		return rhs;
	}
	else
		return ParsingResult<ASTExpr*>(rhs.wasSuccessful());
}

ParsingResult<ASTExpr*> Parser::parsePrefixExpr()
{
	// <prefix_expr>  = <unary_operator> <prefix_expr> | <exp_expr>

	// <unary_operator> <prefix_expr> 
	if (auto matchUop = matchUnaryOp()) // <unary_operator>
	{
		if (auto parseres = parsePrefixExpr())
		{
			return ParsingResult<ASTExpr*>(
				std::make_unique<ASTUnaryExpr>(matchUop.result, std::move(parseres.result))
			);
		}
		else
		{
			errorExpected("Expected an expression after unary operator in prefix expression.");
			return ParsingResult<ASTExpr*>(false);
		}
	}

	// <exp_expr>
	if (auto expExpr = parseExponentExpr())
		return expExpr;
	else
		return ParsingResult<ASTExpr*>(expExpr.wasSuccessful());
}

ParsingResult<ASTExpr*>  Parser::parseCastExpr()
{
	// <cast_expr>  = <prefix_expr> ["as" <type>]
	// <cast_expr>
	if (auto parse_res = parsePrefixExpr())
	{
		// ["as" <type>]
		if (matchKeyword(KeywordType::KW_AS))
		{
			// <type>
			if (auto castType = parseTypeKw())
			{
				return ParsingResult<ASTExpr*>(
						std::make_unique<ASTCastExpr>(castType, std::move(parse_res.result))
					);
			}
			else
			{
				errorExpected("Expected a type keyword after \"as\" in cast expression.");
				return ParsingResult<ASTExpr*>(false);
			}
		}
		return ParsingResult<ASTExpr*>(
				std::move(parse_res.result)
		);
	}
	return ParsingResult<ASTExpr*>();
}

ParsingResult<ASTExpr*>  Parser::parseBinaryExpr(const char & priority)
{
	// <binary_expr>  = <cast_expr> { <binary_operator> <cast_expr> }	
	auto rtr = std::make_unique<ASTBinaryExpr>(binaryOperator::DEFAULT);

	// <cast_expr> OR a binaryExpr of inferior priority.
	ParsingResult<ASTExpr*> lhs_res;
	if (priority > 0)
		lhs_res = parseBinaryExpr(priority - 1);
	else
		lhs_res = parseCastExpr();

	if (!lhs_res)
		return ParsingResult<ASTExpr*>(lhs_res.wasSuccessful());

	rtr->setLHS(std::move(lhs_res.result));	

	// { <binary_operator> <cast_expr> }	
	while (true)
	{
		// <binary_operator>
		auto binop_res = matchBinaryOp(priority);
		if (!binop_res) // No operator found : break.
			break;

		// <cast_expr> OR a binaryExpr of inferior priority.
		ParsingResult<ASTExpr*> rhs_res;
		if (priority > 0)
			rhs_res = parseBinaryExpr(priority - 1);
		else
			rhs_res = parseCastExpr();


		// Handle results appropriately

		if (!rhs_res) // Check for validity : we need a rhs. if we don't have one, we have an error !
		{
			if(rhs_res.wasSuccessful())
				errorExpected("Expected an expression after binary operator,");
			return ParsingResult<ASTExpr*>(lhs_res.wasSuccessful());
		}

		if (rtr->getOp() == binaryOperator::DEFAULT) // if the node has still no operation set, set it
			rtr->setOp(binop_res.result);
		else // else, one up it 
			rtr = oneUpNode(std::move(rtr), binop_res.result);

		rtr->setRHS(std::move(rhs_res.result)); // Set second as the child of the node.
	}

	// When we have simple node (DEFAULT operation with only a value/expr as left child), we simplify it(only return the left child)
	auto simple = rtr->getSimple();
	if (simple)
		return ParsingResult<ASTExpr*>(std::move(simple));
	return ParsingResult<ASTExpr*>(std::move(rtr));
}

ParsingResult<ASTExpr*> Parser::parseExpr()
{
	//  <expr> = <binary_expr> [<assign_operator> <expr>] 
	if (auto lhs_res = parseBinaryExpr())
	{
		auto matchResult = matchAssignOp();
		if (matchResult)
		{
			auto rhs_res = parseExpr();
			if (!rhs_res)
			{
				errorExpected("Expected expression after assignement operator.");
				return ParsingResult<ASTExpr*>(false);
			}

			return ParsingResult<ASTExpr*>(
					std::make_unique<ASTBinaryExpr>(
							matchResult.result,
							std::move(lhs_res.result),
							std::move(rhs_res.result)
						)
				);
		}
		return ParsingResult<ASTExpr*>(std::move(lhs_res.result));
	}
	else 
		return ParsingResult<ASTExpr*>(lhs_res.wasSuccessful()); // return true if lhs_res was simply not found (wasSuccessful returns true), and false if lhs_res had an error.
}

ParsingResult<ASTExpr*> Parser::parseParensExpr(const bool& isMandatory)
{
	// <parens_expr> = '(' <expr> ')'
	// '('
	if (matchSign(SignType::S_ROUND_OPEN))
	{
		std::unique_ptr<ASTExpr> rtr = nullptr;
		// <expr>
		if (auto parseres = parseExpr())
			rtr = std::move(parseres.result);
		else 
		{
			if(parseres.wasSuccessful())
				errorExpected("Expected an expression");
			if (!resyncToSign(SignType::S_ROUND_CLOSE, /*don't consume the ) so it can be picked up below*/ false))
				return ParsingResult<ASTExpr*>(false);
		}

		// ')'
		if (!matchSign(SignType::S_ROUND_CLOSE))
		{
			errorExpected("Expected a ')' ,");
			if (!resyncToSign(SignType::S_ROUND_CLOSE))
				return ParsingResult<ASTExpr*>(false);
			// don't return yet, if we recovered, we can return the expression if there's one
		}
		if(rtr)
			return ParsingResult<ASTExpr*>(std::move(rtr));
		return ParsingResult<ASTExpr*>(false);
	}
	// failure to match ( while expression was mandatory -> try to recover to a ) + error
	else if (isMandatory)
	{
		errorExpected("Expected a '('");
		resyncToSign(SignType::S_ROUND_CLOSE);
		return ParsingResult<ASTExpr*>(false);
	}
	// notfound
	return ParsingResult<ASTExpr*>();
}

ParsingResult<ExprList*> Parser::parseExprList()
{
	// <expr_list> = <expr> {',' <expr> }
	if (auto firstexpr = parseExpr())
	{
		auto exprlist = std::make_unique<ExprList>();
		exprlist->addExpr(std::move(firstexpr.result));
		while (auto comma = matchSign(SignType::S_COMMA))
		{
			if (auto expr = parseExpr())
				exprlist->addExpr(std::move(expr.result));
			else
			{
				if(expr.wasSuccessful())
					errorExpected("Expected an expression");
				return ParsingResult<ExprList*>(false);
			}
		}
		return ParsingResult<ExprList*>(std::move(exprlist));
	}
	return ParsingResult<ExprList*>();
}

ParsingResult<ExprList*> Parser::parseParensExprList()
{
	// <parens_expr_list>	= '(' [ <expr_list> ] ')'
	// '('
	if (auto openPar_res = matchSign(SignType::S_ROUND_OPEN))
	{
		auto exprlist = std::make_unique<ExprList>();
		//  [ <expr_list> ]
		if (auto parsedlist = parseExprList())			
			exprlist = std::move(parsedlist.result);
		else if(!parsedlist.wasSuccessful())
			return ParsingResult<ExprList*>(false);

		// ')'
		if (!matchSign(SignType::S_ROUND_CLOSE))
		{
			errorExpected("Expected a ')'");
			// attempt resync if error
			if (!resyncToSign(SignType::S_ROUND_CLOSE))
				return ParsingResult<ExprList*>(false);
		}
		return ParsingResult<ExprList*>(std::move(exprlist));
	}

	return ParsingResult<ExprList*>();
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
			return ParsingResult<binaryOperator>(binaryOperator::ASSIGN_BASIC);
		restoreParserStateFromBackup(backup);
	}
	return ParsingResult<binaryOperator>();
}

ParsingResult<unaryOperator> Parser::matchUnaryOp()
{
	if (matchSign(SignType::S_EXCL_MARK))
		return ParsingResult<unaryOperator>(unaryOperator::LOGICNOT);
	else if (matchSign(SignType::S_MINUS))
		return ParsingResult<unaryOperator>(unaryOperator::NEGATIVE);
	else if (matchSign(SignType::S_PLUS))
		return ParsingResult<unaryOperator>(unaryOperator::POSITIVE);
	return ParsingResult<unaryOperator>();
}

ParsingResult<binaryOperator> Parser::matchBinaryOp(const char & priority)
{
	auto backup = createParserStateBackup();

	// Check current Token validity, also check if it's a sign because if it isn't we can return directly!
	if (!getToken().isValid() || !getToken().isSign())
		return ParsingResult<binaryOperator>();

	switch (priority)
	{
		case 0: // * / %
			if (matchSign(SignType::S_ASTERISK))
			{
				if (!peekSign(getCurrentPosition(),SignType::S_ASTERISK)) // Disambiguation between '**' and '*'
					return ParsingResult<binaryOperator>(binaryOperator::MUL );
				restoreParserStateFromBackup(backup);; // Backtrack if we didn't return.
			}
			if (matchSign(SignType::S_SLASH))
				return ParsingResult<binaryOperator>(binaryOperator::DIV);
			if (matchSign(SignType::S_PERCENT))
				return ParsingResult<binaryOperator>(binaryOperator::MOD);
			break;
		case 1: // + -
			if (matchSign(SignType::S_PLUS))
				return ParsingResult<binaryOperator>(binaryOperator::ADD );
			if (matchSign(SignType::S_MINUS))
				return ParsingResult<binaryOperator>(binaryOperator::MINUS);
			break;
		case 2: // > >= < <=
			if (matchSign(SignType::S_LESS_THAN))
			{
				if (matchSign(SignType::S_EQUAL))
					return ParsingResult<binaryOperator>(binaryOperator::LESS_OR_EQUAL );
				return ParsingResult<binaryOperator>(binaryOperator::LESS_THAN );
			}
			if (matchSign(SignType::S_GREATER_THAN))
			{
				if (matchSign(SignType::S_EQUAL))
					return ParsingResult<binaryOperator>(binaryOperator::GREATER_OR_EQUAL );
				return ParsingResult<binaryOperator>(binaryOperator::GREATER_THAN );
			}
			break;
		case 3:	// == !=
			// try to match '=' twice.
			if (matchSign(SignType::S_EQUAL))
			{
				if (matchSign(SignType::S_EQUAL))
					return ParsingResult<binaryOperator>(binaryOperator::EQUAL );
				restoreParserStateFromBackup(backup);; // Backtrack if we didn't return.
			}
			if (matchSign(SignType::S_EXCL_MARK))
			{
				if (matchSign(SignType::S_EQUAL))
					return ParsingResult<binaryOperator>(binaryOperator::NOTEQUAL);
				restoreParserStateFromBackup(backup);; // Backtrack if we didn't return.
			}
			break;
		case 4: // &&
			if (matchSign(SignType::S_AMPERSAND))
			{
				if (matchSign(SignType::S_AMPERSAND))
					return ParsingResult<binaryOperator>(binaryOperator::LOGIC_AND);
				restoreParserStateFromBackup(backup);; // Backtrack if we didn't return.
			}
			break;
		case 5: // ||
			if (matchSign(SignType::S_VBAR))
			{
				if (matchSign(SignType::S_VBAR))
					return ParsingResult<binaryOperator>(binaryOperator::LOGIC_OR);
				restoreParserStateFromBackup(backup);; // Backtrack if we didn't return.
			}
			break;
		default:
			throw Exceptions::parser_critical_error("Requested to match a Binary Operator of unknown priority");
			break;
	}
	return ParsingResult<binaryOperator>();
}