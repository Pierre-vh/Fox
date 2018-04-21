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
ParseRes<ASTExpr*> Parser::parseSuffix(std::unique_ptr<ASTExpr>& base)
{
	// <suffix>	 = '.' <decl_call> | '[' <expr> ']'

	// "." <decl_call> 
	// '.'
	if (matchSign(SignType::S_DOT))
	{
		// <decl_call>
		if (auto dc = parseDeclCall())
		{
			// found, return
			return ParseRes<ASTExpr*>(
				std::make_unique<ASTMemberAccessExpr>(std::move(base), std::move(dc.result))
			);
		}
		else 
		{
			// not found
			if (!dc.wasSuccessful())
				errorExpected("Expected an identifier");
			return ParseRes<ASTExpr*>(false);
		}
	}
	// '[' <expr> ']
	// '['
	else if (matchBracket(SignType::S_SQ_OPEN))
	{
		// <expr>
		if (auto expr = parseExpr())
		{
			// ']'
			if (!matchBracket(SignType::S_SQ_CLOSE))
			{
				errorExpected("Expected a ']'");

				if (!resyncToSign(SignType::S_SQ_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
					return ParseRes<ASTDeclRef*>(false);
			}
			return ParseRes<ASTExpr*>(std::make_unique<ASTArrayAccess>(std::move(base), std::move(expr.result)));
		}
		else
		{
			if(expr.wasSuccessful())
				errorExpected("Expected an expression");

			if (resyncToSign(SignType::S_SQ_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
			{
				// Return a node with a null expr, so we return something and avoid error cascades.
				return ParseRes<ASTExpr*>(std::make_unique<ASTArrayAccess>(
						std::move(base),
						std::make_unique<ASTNullExpr>())
					);
			}
			else
			{
				// recovery wasn't successful, return error.
				return ParseRes<ASTExpr*>(false);
			}
		}
	}

	return ParseRes<ASTExpr*>();
}

ParseRes<ASTDeclRef*> Parser::parseDeclCall()
{
	// <decl_call>		= <id> [ <parens_expr_list> ]

	// <id>
	if (auto id = matchID())
	{
		// [ <parens_expr_list> ]
		std::unique_ptr<ASTDeclRef> expr = nullptr;
		if (auto exprlist = parseParensExprList())
		{
			// if an expression list is found create a functioncall node and set expr to that node.
			auto fcall = std::make_unique<ASTFunctionCallExpr>();
			fcall->setFunctionIdentifier(id);
			fcall->setExprList(std::move(exprlist.result));
			expr = std::move(fcall);
		}
		else if(!exprlist.wasSuccessful())
			return ParseRes<ASTDeclRef*>(false);
		else // it's just an identifier
			expr = std::make_unique<ASTDeclRefExpr>(id);
		
		assert(expr && "Expr is null?");
		return ParseRes<ASTDeclRef*>(std::move(expr));
	}
	return ParseRes<ASTDeclRef*>();
}

ParseRes<ASTExpr*> Parser::parsePrimitiveLiteral()
{
	// <primitive_literal>	= One literal of the following type : Integer, Floating-point, Boolean, String, Char
	auto tok = getToken();
	if (tok.isLiteral())
	{
		consumeToken();

		auto litinfo = tok.getLiteralInfo();
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

		return ParseRes<ASTExpr*>(std::move(expr));
	}
	return ParseRes<ASTExpr*>();
}

ParseRes<ASTExpr*> Parser::parseArrayLiteral()
{
	// <array_literal>	= '[' [<expr_list>] ']'
	if (matchBracket(SignType::S_SQ_OPEN))
	{
		auto rtr = std::make_unique<ASTArrayLiteralExpr>();
		// [<expr_list>]
		if (auto elist = parseExprList())
			rtr->setExprList(std::move(elist.result));
		// ']'
		if (!matchBracket(SignType::S_SQ_CLOSE))
		{
			// Resync. If resync wasn't successful, report the error.
			if (!resyncToSign(SignType::S_SQ_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
				return ParseRes<ASTExpr*>(false);
		}
		return ParseRes<ASTExpr*>(std::move(rtr));
	}
	return ParseRes<ASTExpr*>();
}

ParseRes<ASTExpr*> Parser::parseLiteral()
{
	// <literal>	= <primitive_literal> | <array_literal>

	// <primitive_literal>
	if (auto prim = parsePrimitiveLiteral())
		return prim;
	else if (!prim.wasSuccessful())
		return ParseRes<ASTExpr*>(false);

	// <array_literal>
	if (auto arr = parseArrayLiteral())
		return arr;
	else if (!arr.wasSuccessful())
		return ParseRes<ASTExpr*>(false);

	return ParseRes<ASTExpr*>();
}

ParseRes<ASTExpr*>  Parser::parsePrimary()
{
	// = <literal>
	if (auto match_lit = parseLiteral())
		return match_lit;
	else if(!match_lit.wasSuccessful())
		return ParseRes<ASTExpr*>(false);

	// = <decl_call>
	if (auto match_decl = parseDeclCall())
		return ParseRes<ASTExpr*>(std::move(match_decl.result));
	else if(!match_decl.wasSuccessful())
		return ParseRes<ASTExpr*>(false);

	// = '(' <expr> ')'
	if (auto parens_expr = parseParensExpr())
		return parens_expr;
	else if (!parens_expr.wasSuccessful())
		return ParseRes<ASTExpr*>(false);

	return ParseRes<ASTExpr*>();
}

ParseRes<ASTExpr*> Parser::parseArrayOrMemberAccess()
{
	// <array_or_member_access>	= <primary> { <suffix> }
	if (auto prim = parsePrimary())
	{
		std::unique_ptr<ASTExpr> base = std::move(prim.result);
		while (auto suffix = parseSuffix(base))
		{
			// if suffix is usable, assert that lhs is now null
			assert((!base) && "base should have been moved by parseSuffix!");
			assert(suffix.result && "Suffix's result should be null!");
			base = std::move(suffix.result);
		}
		return ParseRes<ASTExpr*>(std::move(base));
	}
	else
		return ParseRes<ASTExpr*>(prim.wasSuccessful());
}

ParseRes<ASTExpr*> Parser::parseExponentExpr()
{
	// <exp_expr>	= <array_or_member_access> [ <exponent_operator> <prefix_expr> ]
	// <member_access>
	if (auto lhs = parseArrayOrMemberAccess())
	{
		// <exponent_operator> 
		if (parseExponentOp())
		{
			// <prefix_expr>
			auto rhs = parsePrefixExpr();
			if (!rhs)
			{
				if(rhs.wasSuccessful())
					errorExpected("Expected an expression after exponent operator.");
				return ParseRes<ASTExpr*>(false);
			}
			return ParseRes<ASTExpr*>(
				std::make_unique<ASTBinaryExpr>(
						binaryOperator::EXP,
						std::move(lhs.result),
						std::move(rhs.result)
					)
			);
		}
		// only <member_access>
		return lhs;
	}
	else
		return ParseRes<ASTExpr*>(lhs.wasSuccessful());
}

ParseRes<ASTExpr*> Parser::parsePrefixExpr()
{
	// <prefix_expr>  = <unary_operator> <prefix_expr> | <exp_expr>

	// <unary_operator> <prefix_expr> 
	if (auto matchUop = parseUnaryOp()) // <unary_operator>
	{
		if (auto parseres = parsePrefixExpr())
		{
			return ParseRes<ASTExpr*>(
				std::make_unique<ASTUnaryExpr>(matchUop.result, std::move(parseres.result))
			);
		}
		else
		{
			if(parseres.wasSuccessful())
				errorExpected("Expected an expression after unary operator in prefix expression.");
			return ParseRes<ASTExpr*>(false);
		}
	}

	// <exp_expr>
	if (auto expExpr = parseExponentExpr())
		return expExpr;
	else
		return ParseRes<ASTExpr*>(expExpr.wasSuccessful());
}

ParseRes<ASTExpr*>  Parser::parseCastExpr()
{
	// <cast_expr>  = <prefix_expr> ["as" <type>]
	// <cast_expr>
	if (auto parse_res = parsePrefixExpr())
	{
		// ["as" <type>]
		if (matchKeyword(KeywordType::KW_AS))
		{
			// <type>
			if (auto castType = parseBuiltinTypename())
			{
				return ParseRes<ASTExpr*>(
						std::make_unique<ASTCastExpr>(castType, std::move(parse_res.result))
					);
			}
			else
			{
				errorExpected("Expected a type keyword after \"as\" in cast expression.");
				return ParseRes<ASTExpr*>(false);
			}
		}
		return ParseRes<ASTExpr*>(
				std::move(parse_res.result)
		);
	}
	else
		return ParseRes<ASTExpr*>(parse_res.wasSuccessful());
}

ParseRes<ASTExpr*> Parser::parseBinaryExpr(const char & priority)
{
	// <binary_expr>  = <cast_expr> { <binary_operator> <cast_expr> }	
	auto rtr = std::make_unique<ASTBinaryExpr>(binaryOperator::DEFAULT);

	// <cast_expr> OR a binaryExpr of inferior priority.
	ParseRes<ASTExpr*> lhs_res;
	if (priority > 0)
		lhs_res = parseBinaryExpr(priority - 1);
	else
		lhs_res = parseCastExpr();

	if (!lhs_res)
		return ParseRes<ASTExpr*>(lhs_res.wasSuccessful());

	rtr->setLHS(std::move(lhs_res.result));	

	// { <binary_operator> <cast_expr> }	
	while (true)
	{
		// <binary_operator>
		auto binop_res = parseBinaryOp(priority);
		if (!binop_res) // No operator found : break.
			break;

		// <cast_expr> OR a binaryExpr of inferior priority.
		ParseRes<ASTExpr*> rhs_res;
		if (priority > 0)
			rhs_res = parseBinaryExpr(priority - 1);
		else
			rhs_res = parseCastExpr();


		// Handle results appropriately

		if (!rhs_res) // Check for validity : we need a rhs. if we don't have one, we have an error !
		{
			if(rhs_res.wasSuccessful())
				errorExpected("Expected an expression after binary operator,");
			return ParseRes<ASTExpr*>(false);
		}

		if (rtr->getOp() == binaryOperator::DEFAULT) // if the node has still no operation set, set it
			rtr->setOp(binop_res.result);
		else 
			rtr = std::make_unique<ASTBinaryExpr>(binop_res.result,std::move(rtr));

		rtr->setRHS(std::move(rhs_res.result)); // Set second as the child of the node.
	}

	// When we have simple node (DEFAULT operation with only a value/expr as left child), we simplify it(only return the left child)
	auto simple = rtr->getSimple();

	if (simple)
		return ParseRes<ASTExpr*>(std::move(simple));
	return ParseRes<ASTExpr*>(std::move(rtr));
}

ParseRes<ASTExpr*> Parser::parseExpr()
{
	//  <expr> = <binary_expr> [<assign_operator> <expr>] 
	if (auto lhs_res = parseBinaryExpr())
	{
		auto matchResult = parseAssignOp();
		if (matchResult)
		{
			auto rhs_res = parseExpr();
			if (!rhs_res)
			{
				if(rhs_res.wasSuccessful())
					errorExpected("Expected expression after assignement operator.");
				return ParseRes<ASTExpr*>(false);
			}

			return ParseRes<ASTExpr*>(
					std::make_unique<ASTBinaryExpr>(
							matchResult.result,
							std::move(lhs_res.result),
							std::move(rhs_res.result)
						)
				);
		}
		return ParseRes<ASTExpr*>(std::move(lhs_res.result));
	}
	else 
		return ParseRes<ASTExpr*>(lhs_res.wasSuccessful()); // return true if lhs_res was simply not found (wasSuccessful returns true), and false if lhs_res had an error.
}

ParseRes<ASTExpr*> Parser::parseParensExpr(const bool& isMandatory)
{
	// <parens_expr> = '(' <expr> ')'
	// '('
	if (matchBracket(SignType::S_ROUND_OPEN))
	{
		std::unique_ptr<ASTExpr> rtr = nullptr;
		// <expr>
		if (auto parseres = parseExpr())
			rtr = std::move(parseres.result);
		else 
		{
			// no expr, handle error & attempt to recover if it's allowed.
			if(parseres.wasSuccessful())
				errorExpected("Expected an expression");
			if (resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
			{
				// Return a null expr in case of a successful recovery.
				rtr = std::make_unique<ASTNullExpr>();
			}
			else
				return ParseRes<ASTExpr*>(false); // return if no resync
		}
		// Normally, the return value shouldn't be null, error should have been handled, but this
		// assert's here for bug-catching purposes.
		assert(rtr && "The return value shouldn't be null at this stage!");

		// ')'
		if (!matchBracket(SignType::S_ROUND_CLOSE))
		{
			// no ), handle error & attempt to recover if it's allowed.
			errorExpected("Expected a ')'");
			if (!resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
			{
				// Couldn't resync successfully, return an error.
				return ParseRes<ASTExpr*>(false);
			}
		}

		return ParseRes<ASTExpr*>(std::move(rtr));
	}
	// failure to match ( while expression was mandatory -> error
	else if (isMandatory)
	{
		errorExpected("Expected a '('");

		// Same thing as parseCompoundStatement, lines 49->56,
		// we attempt recovery, if it's successful, we return a null expr.
		// if it isn't, we backtrack and return "not found".

		// Note : We could totally return an error on both cases, but the current callers
		// don't care if we had a notfound/error and abandon their parsing directly after anyways.

		auto backup = createParserStateBackup();

		if (resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
			return ParseRes<ASTExpr*>(std::make_unique<ASTNullExpr>());

		restoreParserStateFromBackup(backup);

		return ParseRes<ASTExpr*>();
	}
	// notfound
	return ParseRes<ASTExpr*>();
}

ParseRes<ExprList*> Parser::parseExprList()
{
	// <expr_list> = <expr> {',' <expr> }
	if (auto firstexpr = parseExpr())
	{
		auto exprlist = std::make_unique<ExprList>();
		exprlist->addExpr(std::move(firstexpr.result));
		std::size_t latestCommaPosition;
		while (auto comma = matchSign(SignType::S_COMMA))
		{
			latestCommaPosition = getCurrentPosition() - 1;
			if (auto expr = parseExpr())
				exprlist->addExpr(std::move(expr.result));
			else
			{
				if (expr.wasSuccessful())
				{
					// if the expression was just not found, revert the "comma consuming" and
					// let the caller deal with the extra comma.
					setPosition(latestCommaPosition);
					break;
				}
				// else, if there was an error, "propagate" it.
				return ParseRes<ExprList*>(false);
			}
		}
		return ParseRes<ExprList*>(std::move(exprlist));
	}
	return ParseRes<ExprList*>();
}

ParseRes<ExprList*> Parser::parseParensExprList()
{
	// <parens_expr_list>	= '(' [ <expr_list> ] ')'
	// '('
	if (matchBracket(SignType::S_ROUND_OPEN))
	{
		auto exprlist = std::make_unique<ExprList>();
		//  [ <expr_list> ]
		if (auto parsedlist = parseExprList())			
			exprlist = std::move(parsedlist.result);
		else if (!parsedlist.wasSuccessful())		
		{
			// error? Try to recover from it, if success, just discard the expr list,
			// if no success return error.
			if (resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
				return ParseRes<ExprList*>(std::move(exprlist));
			else 
				return ParseRes<ExprList*>(false);
		}

		// ')'
		if (!matchBracket(SignType::S_ROUND_CLOSE))
		{
			errorExpected("Expected a ')'");

			if (!resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
				return ParseRes<ExprList*>(false); // Recovery wasn't successful, return an error.
		}
		return ParseRes<ExprList*>(std::move(exprlist));
	}

	return ParseRes<ExprList*>();
}

bool Parser::parseExponentOp()
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

ParseRes<binaryOperator> Parser::parseAssignOp()
{
	auto backup = createParserStateBackup();
	if (matchSign(SignType::S_EQUAL))
	{
		// Try to match a S_EQUAL. If failed, that means that the next token isn't a =
		// If it succeeds, we founda '==', this is the comparison operator and we must backtrack to prevent errors.
		if (!matchSign(SignType::S_EQUAL))
			return ParseRes<binaryOperator>(binaryOperator::ASSIGN_BASIC);
		restoreParserStateFromBackup(backup);
	}
	return ParseRes<binaryOperator>();
}

ParseRes<unaryOperator> Parser::parseUnaryOp()
{
	if (matchSign(SignType::S_EXCL_MARK))
		return ParseRes<unaryOperator>(unaryOperator::LOGICNOT);
	else if (matchSign(SignType::S_MINUS))
		return ParseRes<unaryOperator>(unaryOperator::NEGATIVE);
	else if (matchSign(SignType::S_PLUS))
		return ParseRes<unaryOperator>(unaryOperator::POSITIVE);
	return ParseRes<unaryOperator>();
}

ParseRes<binaryOperator> Parser::parseBinaryOp(const char & priority)
{
	auto backup = createParserStateBackup();

	// Check current Token validity, also check if it's a sign because if it isn't we can return directly!
	if (!getToken().isValid() || !getToken().isSign())
		return ParseRes<binaryOperator>();

	switch (priority)
	{
		case 0: // * / %
			if (matchSign(SignType::S_ASTERISK))
			{
				if (!matchSign(SignType::S_ASTERISK)) // Disambiguation between '**' and '*'
					return ParseRes<binaryOperator>(binaryOperator::MUL );
				// else, we matched a *. No worries because at the end of the function
				// we backtrack before returning.
			}
			else if (matchSign(SignType::S_SLASH))
				return ParseRes<binaryOperator>(binaryOperator::DIV);
			else if (matchSign(SignType::S_PERCENT))
				return ParseRes<binaryOperator>(binaryOperator::MOD);
			break;
		case 1: // + -
			if (matchSign(SignType::S_PLUS))
				return ParseRes<binaryOperator>(binaryOperator::ADD );
			else if (matchSign(SignType::S_MINUS))
				return ParseRes<binaryOperator>(binaryOperator::MINUS);
			break;
		case 2: // > >= < <=
			if (matchSign(SignType::S_LESS_THAN))
			{
				if (matchSign(SignType::S_EQUAL))
					return ParseRes<binaryOperator>(binaryOperator::LESS_OR_EQUAL );
				return ParseRes<binaryOperator>(binaryOperator::LESS_THAN );
			}
			else if (matchSign(SignType::S_GREATER_THAN))
			{
				if (matchSign(SignType::S_EQUAL))
					return ParseRes<binaryOperator>(binaryOperator::GREATER_OR_EQUAL );
				return ParseRes<binaryOperator>(binaryOperator::GREATER_THAN );
			}
			break;
		case 3:	// == !=
			// try to match '=' twice.
			if (matchSign(SignType::S_EQUAL))
			{
				if (matchSign(SignType::S_EQUAL))
					return ParseRes<binaryOperator>(binaryOperator::EQUAL );
			}
			else if (matchSign(SignType::S_EXCL_MARK))
			{
				if (matchSign(SignType::S_EQUAL))
					return ParseRes<binaryOperator>(binaryOperator::NOTEQUAL);
			}
			break;
		case 4: // &&
			if (matchSign(SignType::S_AMPERSAND))
			{
				if (matchSign(SignType::S_AMPERSAND))
					return ParseRes<binaryOperator>(binaryOperator::LOGIC_AND);
			}
			break;
		case 5: // ||
			if (matchSign(SignType::S_VBAR))
			{
				if (matchSign(SignType::S_VBAR))
					return ParseRes<binaryOperator>(binaryOperator::LOGIC_OR);
			}
			break;
		default:
			throw Exceptions::parser_critical_error("Requested to match a Binary Operator of unknown priority");
			break;
	}
	restoreParserStateFromBackup(backup);; // Backtrack if we didn't return.
	return ParseRes<binaryOperator>();
}