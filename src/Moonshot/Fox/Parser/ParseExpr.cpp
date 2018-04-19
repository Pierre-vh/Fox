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
ParsingResult<ASTExpr*> Parser::parseSuffix(std::unique_ptr<ASTExpr>& base)
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
			return ParsingResult<ASTExpr*>(
				std::make_unique<ASTMemberAccessExpr>(std::move(base), std::move(dc.result))
			);
		}
		else 
		{
			// not found
			if (!dc.wasSuccessful())
				errorExpected("Expected an identifier");
			return ParsingResult<ASTExpr*>(false);
		}
	}
	// '[' <expr> ']
	// '['
	else if (matchSign(SignType::S_SQ_OPEN))
	{
		// <expr>
		if (auto expr = parseExpr())
		{
			// ']'
			if (!matchSign(SignType::S_SQ_CLOSE))
			{
				errorExpected("Expected a ']'");
				// try recovery if possible. if failed to recover, return.
				if (!resyncToSignInStatement(SignType::S_SQ_CLOSE))
					return ParsingResult<IASTDeclRef*>(false);
			}
			return ParsingResult<ASTExpr*>(std::make_unique<ASTArrayAccess>(std::move(base), std::move(expr.result)));
		}
		else
		{
			if(expr.wasSuccessful())
				errorExpected("Expected an expression");

			if (resyncToSignInStatement(SignType::S_SQ_CLOSE))
			{
				// Return a node with a null expr, so we return something and avoid error cascades.
				return ParsingResult<ASTExpr*>(std::make_unique<ASTArrayAccess>(
						std::move(base),
						std::make_unique<ASTNullExpr>())
					);
			}
			else
			{
				// recovery wasn't successful, return error.
				return ParsingResult<ASTExpr*>(false);
			}
		}
	}

	return ParsingResult<ASTExpr*>();
}

ParsingResult<IASTDeclRef*> Parser::parseDeclCall()
{
	// <decl_call>		= <id> [ <parens_expr_list> ]

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
		else if(!exprlist.wasSuccessful())
			return ParsingResult<IASTDeclRef*>(false);
		else // it's just an identifier
			expr = std::make_unique<ASTDeclRefExpr>(id);
		
		assert(expr && "Expr is null?");
		return ParsingResult<IASTDeclRef*>(std::move(expr));
	}
	return ParsingResult<IASTDeclRef*>();
}

ParsingResult<ASTExpr*> Parser::parsePrimitiveLiteral()
{
	// <primitive_literal>	= One literal of the following type : Integer, Floating-point, Boolean, String, Char
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

ParsingResult<ASTExpr*> Parser::parseArrayLiteral()
{
	// <array_literal>	= '[' [<expr_list>] ']'
	if (matchSign(SignType::S_SQ_OPEN))
	{
		auto rtr = std::make_unique<ASTArrayLiteralExpr>();
		// [<expr_list>]
		if (auto elist = parseExprList())
			rtr->setExprList(std::move(elist.result));
		// ']'
		if (!matchSign(SignType::S_SQ_CLOSE))
		{
			// Resync. If resync wasn't successful, report the error.
			if (!resyncToSignInStatement(SignType::S_SQ_CLOSE))
				return ParsingResult<ASTExpr*>(false);
		}
		return ParsingResult<ASTExpr*>(std::move(rtr));
	}
	return ParsingResult<ASTExpr*>();
}

ParsingResult<ASTExpr*> Parser::parseLiteral()
{
	// <literal>	= <primitive_literal> | <array_literal>

	// <primitive_literal>
	if (auto prim = parsePrimitiveLiteral())
		return prim;
	else if (!prim.wasSuccessful())
		return ParsingResult<ASTExpr*>(false);

	// <array_literal>
	if (auto arr = parseArrayLiteral())
		return arr;
	else if (!arr.wasSuccessful())
		return ParsingResult<ASTExpr*>(false);

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

ParsingResult<ASTExpr*> Parser::parseArrayOrMemberAccess()
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
		return ParsingResult<ASTExpr*>(std::move(base));
	}
	else
		return ParsingResult<ASTExpr*>(prim.wasSuccessful());
}

ParsingResult<ASTExpr*> Parser::parseExponentExpr()
{
	// <exp_expr>	= <array_or_member_access> [ <exponent_operator> <prefix_expr> ]
	// <member_access>
	if (auto lhs = parseArrayOrMemberAccess())
	{
		// <exponent_operator> 
		if (matchExponentOp())
		{
			// <prefix_expr>
			auto rhs = parsePrefixExpr();
			if (!rhs)
			{
				if(rhs.wasSuccessful())
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
		return lhs;
	}
	else
		return ParsingResult<ASTExpr*>(lhs.wasSuccessful());
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
			if(parseres.wasSuccessful())
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
			if (auto castType = parseBuiltinTypename())
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
	else
		return ParsingResult<ASTExpr*>(parse_res.wasSuccessful());
}

ParsingResult<ASTExpr*> Parser::parseBinaryExpr(const char & priority)
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
			return ParsingResult<ASTExpr*>(false);
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
				if(rhs_res.wasSuccessful())
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
			// no expr, handle error & attempt to recover if it's allowed.
			if(parseres.wasSuccessful())
				errorExpected("Expected an expression");
			if (resyncToSignInStatement(SignType::S_ROUND_CLOSE,false /* don't consume the token so it's picked up below */)) 
			{
				// Return a null expr in case of a successful recovery.
				rtr = std::make_unique<ASTNullExpr>();
			}
			else
				return ParsingResult<ASTExpr*>(false); // return if no resync
		}
		// Normally, the return value shouldn't be null, error should have been handled, but this
		// assert's here for bug-catching purposes.
		assert(rtr && "The return value shouldn't be null at this stage!");

		// ')'
		if (!matchSign(SignType::S_ROUND_CLOSE))
		{
			// no ), handle error & attempt to recover if it's allowed.
			errorExpected("Expected a ')'");
			if (!resyncToSignInStatement(SignType::S_ROUND_CLOSE))
			{
				// Couldn't resync successfully, return an error.
				return ParsingResult<ASTExpr*>(false);
			}
		}

		return ParsingResult<ASTExpr*>(std::move(rtr));
	}
	// failure to match ( while expression was mandatory -> error
	else if (isMandatory)
	{
		errorExpected("Expected a '('");

		// Same thing as parseCompoundStatement, lines 49->56,
		// we attempt recovery, if it's successful, we return a null expr.
		// if it wasn't, we backtrack and return "not found".

		auto backup = createParserStateBackup();
		auto resyncres = resyncToSignInStatement(SignType::S_ROUND_CLOSE);

		if (resyncres.hasRecoveredOnRequestedToken())
			return ParsingResult<ASTExpr*>(std::make_unique<ASTNullExpr>());

		restoreParserStateFromBackup(backup);

		return ParsingResult<ASTExpr*>();
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
		else if (!parsedlist.wasSuccessful())		
		{
			// error? Try to recover from it, if success, just discard the expr list,
			// if no success return error.
			if (resyncToSignInStatement(SignType::S_ROUND_CLOSE))
				return ParsingResult<ExprList*>(std::move(exprlist));
			else 
				return ParsingResult<ExprList*>(false);
		}

		// ')'
		if (!matchSign(SignType::S_ROUND_CLOSE))
		{
			errorExpected("Expected a ')'");

			if (!resyncToSignInStatement(SignType::S_ROUND_CLOSE))
				return ParsingResult<ExprList*>(false); // Recovery wasn't successful, return an error.
			// Recovery was successful, just return.
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
			}
			if (matchSign(SignType::S_EXCL_MARK))
			{
				if (matchSign(SignType::S_EQUAL))
					return ParsingResult<binaryOperator>(binaryOperator::NOTEQUAL);
			}
			break;
		case 4: // &&
			if (matchSign(SignType::S_AMPERSAND))
			{
				if (matchSign(SignType::S_AMPERSAND))
					return ParsingResult<binaryOperator>(binaryOperator::LOGIC_AND);
			}
			break;
		case 5: // ||
			if (matchSign(SignType::S_VBAR))
			{
				if (matchSign(SignType::S_VBAR))
					return ParsingResult<binaryOperator>(binaryOperator::LOGIC_OR);
			}
			break;
		default:
			throw Exceptions::parser_critical_error("Requested to match a Binary Operator of unknown priority");
			break;
	}
	restoreParserStateFromBackup(backup);; // Backtrack if we didn't return.
	return ParsingResult<binaryOperator>();
}