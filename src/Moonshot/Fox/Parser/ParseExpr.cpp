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

#include "Moonshot/Fox/Common/Exceptions.hpp"
#include "Moonshot/Fox/AST/ASTExpr.hpp"
#include <cassert>

using namespace Moonshot;

// note, here the unique_ptr is passed by reference because it will be consumed (moved -> nulled) only if a '[' is found, else, it's left untouched.
Parser::ExprResult Parser::parseSuffix(std::unique_ptr<ASTExpr>& base)
{
	// <suffix>	 = '.' <decl_call> | '[' <expr> ']'

	// "." <decl_call> 
	// '.'
	if (consumeSign(SignType::S_DOT))
	{
		// <decl_call>
		if (auto decl_call = parseDeclCall())
		{
			// found, return
			return ExprResult(
				std::make_unique<ASTMemberAccessExpr>(std::move(base), decl_call.moveAs<ASTDeclRef>())
			);
		}
		else 
		{
			// not found
			if (!decl_call.wasSuccessful())
				errorExpected("Expected an identifier");
			return ExprResult::Error();
		}
	}
	// '[' <expr> ']
	// '['
	else if (consumeBracket(SignType::S_SQ_OPEN))
	{
		// <expr>
		if (auto expr = parseExpr())
		{
			// ']'
			if (!consumeBracket(SignType::S_SQ_CLOSE))
			{
				errorExpected("Expected a ']'");

				if (!resyncToSign(SignType::S_SQ_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
					return ExprResult::Error();
			}
			return ExprResult(std::make_unique<ASTArrayAccess>(std::move(base), expr.move()));
		}
		else
		{
			if(expr.wasSuccessful())
				errorExpected("Expected an expression");

			if (resyncToSign(SignType::S_SQ_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
			{
				// Return a node with a null expr, so we return something and avoid error cascades.
				return ExprResult(std::make_unique<ASTArrayAccess>(
						std::move(base),
						std::make_unique<ASTNullExpr>())
					);
			}
			else
			{
				// recovery wasn't successful, return error.
				return ExprResult::Error();
			}
		}
	}

	return ExprResult::NotFound();
}

Parser::ExprResult Parser::parseDeclCall()
{
	// <decl_call>		= <id> [ <parens_expr_list> ]

	// <id>
	if (auto id = consumeIdentifier())
	{
		// [ <parens_expr_list> ]
		std::unique_ptr<ASTDeclRef> expr = nullptr;
		if (auto exprlist = parseParensExprList())
		{
			// if an expression list is found create a functioncall node and set expr to that node.
			auto fcall = std::make_unique<ASTFunctionCallExpr>();
			fcall->setFunctionIdentifier(id);
			fcall->setExprList(exprlist.move());
			expr = std::move(fcall);
		}
		else if (!exprlist.wasSuccessful())
			return ExprResult::Error();
		else // not expr list -> it's just an identifier!
			expr = std::make_unique<ASTDeclRefExpr>(id);
		
		assert(expr && "Expr is null?");
		return ExprResult(std::move(expr));
	}
	return ExprResult::NotFound();
}

Parser::ExprResult Parser::parsePrimitiveLiteral()
{
	// <primitive_literal>	= One literal of the following type : Integer, Floating-point, Boolean, String, Char
	auto tok = getCurtok();
	if (tok.isLiteral())
	{
		skipToken();

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

		return ExprResult(std::move(expr));
	}
	return ExprResult::NotFound();
}

Parser::ExprResult Parser::parseArrayLiteral()
{
	// <array_literal>	= '[' [<expr_list>] ']'
	if (consumeBracket(SignType::S_SQ_OPEN))
	{
		auto rtr = std::make_unique<ASTArrayLiteralExpr>();
		// [<expr_list>]
		if (auto elist = parseExprList())
			rtr->setExprList(elist.move());
		// ']'
		if (!consumeBracket(SignType::S_SQ_CLOSE))
		{
			// Resync. If resync wasn't successful, report the error.
			if (!resyncToSign(SignType::S_SQ_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
				return ExprResult::Error();
		}
		return ExprResult(std::move(rtr));
	}
	return ExprResult::NotFound();
}

Parser::ExprResult Parser::parseLiteral()
{
	// <literal>	= <primitive_literal> | <array_literal>

	// <primitive_literal>
	if (auto prim = parsePrimitiveLiteral())
		return prim;
	else if (!prim.wasSuccessful())
		return ExprResult::Error();

	// <array_literal>
	if (auto arr = parseArrayLiteral())
		return arr;
	else if (!arr.wasSuccessful())
		return ExprResult::Error();

	return ExprResult::NotFound();
}

Parser::ExprResult Parser::parsePrimary()
{
	// = <literal>
	if (auto lit = parseLiteral())
		return lit;
	else if(!lit.wasSuccessful())
		return ExprResult::Error();

	// = <decl_call>
	if (auto declcall = parseDeclCall())
		return declcall;
	else if(!declcall.wasSuccessful())
		return ExprResult::Error();

	// = '(' <expr> ')'
	if (auto parens_expr = parseParensExpr())
		return parens_expr;
	else if (!parens_expr.wasSuccessful())
		return ExprResult::Error();

	return ExprResult::NotFound();
}

Parser::ExprResult Parser::parseArrayOrMemberAccess()
{
	// <array_or_member_access>	= <primary> { <suffix> }
	if (auto prim = parsePrimary())
	{
		std::unique_ptr<ASTExpr> base = prim.move();
		while (auto suffix = parseSuffix(base))
		{
			// if suffix is usable, assert that lhs is now null
			assert((!base) && "base should have been moved by parseSuffix!");
			base = std::move(suffix.move());
		}
		return ExprResult(std::move(base));
	}
	else
	{
		if (!prim.wasSuccessful())
			return ExprResult::Error();
		return ExprResult::NotFound();
	}
}

Parser::ExprResult Parser::parseExponentExpr()
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
				return ExprResult::Error();
			}
			return ExprResult(
				std::make_unique<ASTBinaryExpr>(
						binaryOperator::EXP,
						lhs.move(),
						rhs.move()
					)
			);
		}
		// only <member_access>
		return lhs;
	}
	else
	{
		if (!lhs.wasSuccessful())
			return ExprResult::Error();
		return ExprResult::NotFound();
	}
}

Parser::ExprResult Parser::parsePrefixExpr()
{
	// <prefix_expr>  = <unary_operator> <prefix_expr> | <exp_expr>

	// <unary_operator> <prefix_expr> 
	if (auto uop = parseUnaryOp()) // <unary_operator>
	{
		if (auto prefixexpr = parsePrefixExpr())
		{
			return ExprResult(
				std::make_unique<ASTUnaryExpr>(uop.get(),prefixexpr.move())
			);
		}
		else
		{
			if(prefixexpr.wasSuccessful())
				errorExpected("Expected an expression after unary operator in prefix expression.");
			return ExprResult::Error();
		}
	}

	// <exp_expr>
	if (auto expExpr = parseExponentExpr())
		return expExpr;
	else if (!expExpr.wasSuccessful())
		return ExprResult::Error();

	return ExprResult::NotFound();
}

Parser::ExprResult Parser::parseCastExpr()
{
	// <cast_expr>  = <prefix_expr> ["as" <type>]
	// <cast_expr>
	if (auto prefixexpr = parsePrefixExpr())
	{
		// ["as" <type>]
		if (consumeKeyword(KeywordType::KW_AS))
		{
			// <type>
			if (auto castType = parseBuiltinTypename())
			{
				return ExprResult(
						std::make_unique<ASTCastExpr>(castType,prefixexpr.move())
					);
			}
			else
			{
				errorExpected("Expected a type keyword after \"as\" in cast expression.");
				return ExprResult::Error();
			}
		}
		return ExprResult(
			prefixexpr.move()
		);
	}
	else
	{
		if (!prefixexpr.wasSuccessful())
			return ExprResult::Error();
		return ExprResult::NotFound();
	}
}

Parser::ExprResult Parser::parseBinaryExpr(const char & priority)
{
	// <binary_expr>  = <cast_expr> { <binary_operator> <cast_expr> }	
	auto rtr = std::make_unique<ASTBinaryExpr>(binaryOperator::DEFAULT);

	// <cast_expr> OR a binaryExpr of inferior priority.
	ExprResult lhs;
	if (priority > 0)
		lhs = parseBinaryExpr(priority - 1);
	else
		lhs = parseCastExpr();

	if (!lhs)
	{
		if (!lhs.wasSuccessful())
			return ExprResult::Error();
		return ExprResult::NotFound();
	}

	rtr->setLHS(lhs.move());

	// { <binary_operator> <cast_expr> }	
	while (true)
	{
		// <binary_operator>
		auto binop_res = parseBinaryOp(priority);
		if (!binop_res) // No operator found : break.
			break;

		// <cast_expr> OR a binaryExpr of inferior priority.
		ExprResult rhs;
		if (priority > 0)
			rhs = parseBinaryExpr(priority - 1);
		else
			rhs = parseCastExpr();


		// Handle results appropriately

		if (!rhs) // Check for validity : we need a rhs. if we don't have one, we have an error !
		{
			if(rhs.wasSuccessful())
				errorExpected("Expected an expression after binary operator,");
			return ExprResult::Error();
		}

		if (rtr->getOp() == binaryOperator::DEFAULT) // if the node has still no operation set, set it
			rtr->setOp(binop_res.get());
		else 
			rtr = std::make_unique<ASTBinaryExpr>(binop_res.get(),std::move(rtr));

		rtr->setRHS(rhs.move()); // Set second as the child of the node.
	}

	// When we have simple node (DEFAULT operation with only a value/expr as left child), we simplify it(only return the left child)
	auto simple = rtr->getSimple();

	if (simple)
		return ExprResult(std::move(simple));
	return ExprResult(std::move(rtr));
}

Parser::ExprResult Parser::parseExpr()
{
	//  <expr> = <binary_expr> [<assign_operator> <expr>] 
	if (auto lhs = parseBinaryExpr())
	{
		if (auto op = parseAssignOp())
		{
			auto rhs = parseExpr();
			if (!rhs)
			{
				if(rhs.wasSuccessful())
					errorExpected("Expected expression after assignement operator.");
				return ExprResult::Error();
			}

			return ExprResult(
					std::make_unique<ASTBinaryExpr>(
							op.get(),
							lhs.move(),
							rhs.move()
						)
				);
		}
		return ExprResult(lhs.move());
	}
	else
	{
		if (!lhs.wasSuccessful())
			return ExprResult::Error();
		return ExprResult::NotFound();
	}
}

Parser::ExprResult Parser::parseParensExpr(const bool& isMandatory)
{
	// <parens_expr> = '(' <expr> ')'
	// '('
	if (consumeBracket(SignType::S_ROUND_OPEN))
	{
		std::unique_ptr<ASTExpr> rtr = nullptr;
		
		// <expr>
		if (auto expr = parseExpr())
			rtr = expr.move();
		else 
		{
			// no expr, handle error & attempt to recover if it's allowed.
			if(expr.wasSuccessful())
				errorExpected("Expected an expression");
			if (resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ false))
			{
				// Return a null expr in case of a successful recovery.
				rtr = std::make_unique<ASTNullExpr>();
			}
			else
				return ExprResult::Error();
		}
		// Normally, the return value shouldn't be null, error should have been handled, but this
		// assert's here for bug-catching purposes.
		assert(rtr && "The return value shouldn't be null at this stage!");

		// ')'
		if (!consumeBracket(SignType::S_ROUND_CLOSE))
		{
			// no ), handle error & attempt to recover if it's allowed.
			errorExpected("Expected a ')'");
			if (!resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
				return ExprResult::Error();
		}

		return ExprResult(std::move(rtr));
	}
	// failure to match ( while expression was mandatory -> error
	else if (isMandatory)
	{
		errorExpected("Expected a '('");

		// Same thing as parseCompoundStatemen
		// we attempt recovery, if it's successful, we return a null expr.
		// if it isn't, we backtrack and return "not found". (since the CompoundStatement is entirely missing)

		// Note : We could totally return an error on both cases, but the current callers
		// don't care if we had a notfound/error and abandon their parsing directly after anyways.

		auto backup = createParserStateBackup();

		if (resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
			return ExprResult(std::make_unique<ASTNullExpr>());

		restoreParserStateFromBackup(backup);
	}
	return ExprResult::NotFound();
}

Parser::ExprListResult Parser::parseExprList()
{
	// <expr_list> = <expr> {',' <expr> }
	if (auto firstexpr = parseExpr())
	{
		auto exprlist = std::make_unique<ASTExprList>();
		exprlist->addExpr(firstexpr.move());
		while (auto comma = consumeSign(SignType::S_COMMA))
		{
			if (auto expr = parseExpr())
				exprlist->addExpr(expr.move());
			else
			{
				if (expr.wasSuccessful())
				{
					// if the expression was just not found, revert the comma consuming and
					// let the caller deal with the extra comma after the expression list.
					revertConsume();
					break;
				}

				return ExprListResult::Error();
			}
		}
		return ExprListResult(std::move(exprlist));
	}
	return ExprListResult::NotFound();
}

Parser::ExprListResult Parser::parseParensExprList()
{
	// <parens_expr_list>	= '(' [ <expr_list> ] ')'
	// '('
	if (consumeBracket(SignType::S_ROUND_OPEN))
	{
		auto expr_list = std::make_unique<ASTExprList>();
		//  [ <expr_list> ]
		if (auto elist = parseExprList())
			elist = elist.move();
		else if (!elist.wasSuccessful())
		{
			// error? Try to recover from it, if success, just discard the expr list,
			// if no success return error.
			if (resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
				return ExprListResult(std::make_unique<ASTExprList>()); // if recovery is successful, return an empty expression list.
			else
				return ExprListResult::Error();
		}

		// ')'
		if (!consumeBracket(SignType::S_ROUND_CLOSE))
		{
			errorExpected("Expected a ')'");

			if (!resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
				return ExprListResult::Error(); 
		}
		return ExprListResult(std::move(expr_list));
	}

	return ExprListResult::NotFound();
}

bool Parser::parseExponentOp()
{
	auto backup = createParserStateBackup();
	if (consumeSign(SignType::S_ASTERISK))
	{
		if (consumeSign(SignType::S_ASTERISK))
			return true;
		restoreParserStateFromBackup(backup);
	}
	return false;
}

Parser::Result<binaryOperator> Parser::parseAssignOp()
{
	auto backup = createParserStateBackup();
	if (consumeSign(SignType::S_EQUAL))
	{
		// Try to match a S_EQUAL. If failed, that means that the next token isn't a =
		// If it succeeds, we founda '==', this is the comparison operator and we must backtrack to prevent errors.
		if (!consumeSign(SignType::S_EQUAL))
			return Result<binaryOperator>(binaryOperator::ASSIGN_BASIC);
		restoreParserStateFromBackup(backup);
	}
	return Result<binaryOperator>::NotFound();
}

Parser::Result<unaryOperator> Parser::parseUnaryOp()
{
	if (consumeSign(SignType::S_EXCL_MARK))
		return Result<unaryOperator>(unaryOperator::LOGICNOT);
	else if (consumeSign(SignType::S_MINUS))
		return Result<unaryOperator>(unaryOperator::NEGATIVE);
	else if (consumeSign(SignType::S_PLUS))
		return Result<unaryOperator>(unaryOperator::POSITIVE);
	return Result<unaryOperator>::NotFound();
}

Parser::Result<binaryOperator> Parser::parseBinaryOp(const char & priority)
{
	// Check current Token validity, also check if it's a sign because if it isn't we can return directly!
	if (!getCurtok().isValid() || !getCurtok().isSign())
		return Result<binaryOperator>::NotFound();

	auto backup = createParserStateBackup();

	switch (priority)
	{
		case 0: // * / %
			if (consumeSign(SignType::S_ASTERISK))
			{
				if (!consumeSign(SignType::S_ASTERISK)) // Disambiguation between '**' and '*'
					return Result<binaryOperator>(binaryOperator::MUL );
			}
			else if (consumeSign(SignType::S_SLASH))
				return Result<binaryOperator>(binaryOperator::DIV);
			else if (consumeSign(SignType::S_PERCENT))
				return Result<binaryOperator>(binaryOperator::MOD);
			break;
		case 1: // + -
			if (consumeSign(SignType::S_PLUS))
				return Result<binaryOperator>(binaryOperator::ADD );
			else if (consumeSign(SignType::S_MINUS))
				return Result<binaryOperator>(binaryOperator::MINUS);
			break;
		case 2: // > >= < <=
			if (consumeSign(SignType::S_LESS_THAN))
			{
				if (consumeSign(SignType::S_EQUAL))
					return Result<binaryOperator>(binaryOperator::LESS_OR_EQUAL );
				return Result<binaryOperator>(binaryOperator::LESS_THAN );
			}
			else if (consumeSign(SignType::S_GREATER_THAN))
			{
				if (consumeSign(SignType::S_EQUAL))
					return Result<binaryOperator>(binaryOperator::GREATER_OR_EQUAL );
				return Result<binaryOperator>(binaryOperator::GREATER_THAN );
			}
			break;
		case 3:	// == !=
			// try to match '=' twice.
			if (consumeSign(SignType::S_EQUAL))
			{
				if (consumeSign(SignType::S_EQUAL))
					return Result<binaryOperator>(binaryOperator::EQUAL );
			}
			else if (consumeSign(SignType::S_EXCL_MARK))
			{
				if (consumeSign(SignType::S_EQUAL))
					return Result<binaryOperator>(binaryOperator::NOTEQUAL);
			}
			break;
		case 4: // &&
			if (consumeSign(SignType::S_AMPERSAND))
			{
				if (consumeSign(SignType::S_AMPERSAND))
					return Result<binaryOperator>(binaryOperator::LOGIC_AND);
			}
			break;
		case 5: // ||
			if (consumeSign(SignType::S_VBAR))
			{
				if (consumeSign(SignType::S_VBAR))
					return Result<binaryOperator>(binaryOperator::LOGIC_OR);
			}
			break;
		default:
			throw Exceptions::parser_critical_error("Requested to match a Binary Operator of unknown priority");
			break;
	}
	restoreParserStateFromBackup(backup);; // Backtrack if we didn't return.
	return Result<binaryOperator>::NotFound();
}