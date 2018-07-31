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

using namespace fox;

Parser::ExprResult Parser::parseSuffix(Expr* base)
{
	// <suffix> = '.' <id> | '[' <expr> ']' | <parens_expr_list>
	SourceLoc begLoc = base->getBegLoc();
	SourceLoc endLoc;
	// "." <id> 
	// '.'
	if (auto dotLoc = consumeSign(SignType::S_DOT))
	{
		// <id>
		if (auto id = consumeIdentifier())
		{
			// found, return
			endLoc = id.getSourceRange().makeEndSourceLoc();
			return ExprResult(
				new(ctxt_) MemberOfExpr(base ,id.get(),begLoc,dotLoc,endLoc)
			);
		}
		else 
		{
			reportErrorExpected(DiagID::parser_expected_iden);
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
			endLoc = consumeBracket(SignType::S_SQ_CLOSE);
			if (!endLoc)
			{
				reportErrorExpected(DiagID::parser_expected_closing_squarebracket);

				if (resyncToSign(SignType::S_SQ_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ false))
					endLoc = consumeBracket(SignType::S_SQ_CLOSE);
				else
					return ExprResult::Error();

			}

			return ExprResult(
				new(ctxt_) ArrayAccessExpr(base, expr.get(), begLoc, endLoc)
			);
		}
		else
		{
			if (expr.wasSuccessful())
				reportErrorExpected(DiagID::parser_expected_expr);

			// Resync. if Resync is successful, return the base as the result (don't alter it) to fake a success
			// , if it's not, return an Error.
			if (resyncToSign(SignType::S_SQ_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
				return ExprResult(base);
			else
				return ExprResult::Error();
		}
	}
	// <parens_expr_list>
	else if (auto exprlist = parseParensExprList(nullptr,&endLoc))
	{
		assert(endLoc && "parseParensExprList didn't complete the endLoc?");
		return ExprResult(
			new(ctxt_) FunctionCallExpr(base, std::move(exprlist.get()), begLoc,endLoc)
		);
	}
	else if (!exprlist.wasSuccessful())
		return ExprResult::Error();
	return ExprResult::NotFound();
}

Parser::ExprResult Parser::parseDeclRef()
{
	// Note: this rule is quite simple and is essentially just a "wrapper"
	// however I'm keeping it for clarity, and future usages where DeclRef might get more complex, if it ever 
	// does

	// <decl_call> = <id> 
	if (auto id = consumeIdentifier())
		return ExprResult(new(ctxt_) DeclRefExpr(
				id.get(),
				id.getSourceRange().getBeginSourceLoc(),
				id.getSourceRange().makeEndSourceLoc()
			));
	return ExprResult::NotFound();
}

Parser::ExprResult Parser::parsePrimitiveLiteral()
{
	// <primitive_literal>	= One literal of the following type : Integer, Floating-point, Boolean, String, Char
	auto tok = getCurtok();
	if (!tok.isLiteral())
		return ExprResult::NotFound();
	
	increaseTokenIter();

	auto litinfo = tok.getLiteralInfo();
	Expr* expr = nullptr;

	SourceLoc begLoc = tok.getRange().getBeginSourceLoc();
	SourceLoc endLoc = tok.getRange().makeEndSourceLoc();

	if (litinfo.isBool())
		expr = new(ctxt_) BoolLiteralExpr(litinfo.get<bool>(), begLoc, endLoc);
	else if (litinfo.isString())
		expr = new(ctxt_) StringLiteralExpr(litinfo.get<std::string>(), begLoc, endLoc);
	else if (litinfo.isChar())
		expr = new(ctxt_) CharLiteralExpr(litinfo.get<CharType>(), begLoc, endLoc);
	else if (litinfo.isInt())
		expr = new(ctxt_) IntegerLiteralExpr(litinfo.get<IntType>(), begLoc, endLoc);
	else if (litinfo.isFloat())
		expr = new(ctxt_) FloatLiteralExpr(litinfo.get<FloatType>(), begLoc, endLoc);
	else
		fox_unreachable("Unknown literal kind"); // Unknown literal

	return ExprResult(expr);
}

Parser::ExprResult Parser::parseArrayLiteral()
{
	// <array_literal>	= '[' [<expr_list>] ']'
	auto begLoc = consumeBracket(SignType::S_SQ_OPEN);
	if (!begLoc)
		return ExprResult::NotFound();
	
	SourceLoc endLoc;
	// [<expr_list>]
	auto elist = parseExprList();
	#pragma message("No error checking for the elist, fix that")
	// ']'
	if (!(endLoc = consumeBracket(SignType::S_SQ_CLOSE)))
	{
		// Resync. If resync wasn't successful, report the error.
		if (resyncToSign(SignType::S_SQ_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ false))
			endLoc = consumeBracket(SignType::S_SQ_CLOSE);
		else
			return ExprResult::Error();
	}
	return ExprResult(
		new(ctxt_) ArrayLiteralExpr(std::move(elist.get()) ,begLoc, endLoc)
	);
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
	if (auto declcall = parseDeclRef())
		return declcall;
	else if(!declcall.wasSuccessful())
		return ExprResult::Error();

	// = '(' <expr> ')'
	if (auto parens_expr = parseParensExpr(false))
		return parens_expr;
	else if (!parens_expr.wasSuccessful())
		return ExprResult::Error();

	return ExprResult::NotFound();
}

Parser::ExprResult Parser::parseSuffixExpr()
{
	// <suffix_expr>	= <primary> { <suffix> }
	if (auto prim = parsePrimary())
	{
		Expr* base = prim.get();
		ExprResult suffix;
		while (suffix = parseSuffix(base))
			base = suffix.get();

		if (suffix.wasSuccessful())
			return ExprResult(base);
		else
			return ExprResult::Error();
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
	// <exp_expr>	= <suffix_expr> [ <exponent_operator> <prefix_expr> ]

	// <suffix_expr>
	auto lhs = parseSuffixExpr();
	if (!lhs)
		return lhs; 

	// <exponent_operator> 
	if (auto expOp = parseExponentOp())
	{
		// <prefix_expr>
		auto rhs = parsePrefixExpr();
		if (!rhs)
		{
			if(rhs.wasSuccessful())
				reportErrorExpected(DiagID::parser_expected_expr);
				
			return ExprResult::Error();
		}

		SourceLoc begLoc = lhs.get()->getBegLoc();
		SourceLoc endLoc = lhs.get()->getEndLoc();

		return ExprResult(
			new(ctxt_) BinaryExpr(
					BinaryOperator::EXP,
					lhs.get(),
					rhs.get(),
					begLoc,
					expOp, 
					endLoc
			));
	}

	return lhs;
}

Parser::ExprResult Parser::parsePrefixExpr()
{
	// <prefix_expr>  = <unary_operator> <prefix_expr> | <exp_expr>

	// <unary_operator> <prefix_expr> 
	if (auto uop = parseUnaryOp()) // <unary_operator>
	{
		if (auto prefixexpr = parsePrefixExpr())
		{
			SourceLoc endLoc = prefixexpr.get()->getEndLoc();
			return ExprResult(
				new(ctxt_) UnaryExpr(
					uop.get(),
					prefixexpr.get(),
					uop.getSourceRange().getBeginSourceLoc(),
					uop.getSourceRange(),
					endLoc
				)
			);
		}
		else
		{
			if(prefixexpr.wasSuccessful())
				reportErrorExpected(DiagID::parser_expected_expr);

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
	auto prefixexpr = parsePrefixExpr();
	if (!prefixexpr)
	{
		if (!prefixexpr.wasSuccessful())
			return ExprResult::Error();
		return ExprResult::NotFound();
	}

	// ["as" <type>]
	if (consumeKeyword(KeywordType::KW_AS))
	{
		// <type>
		if (auto castType = parseBuiltinTypename())
		{
			SourceLoc begLoc = prefixexpr.get()->getBegLoc();
			SourceLoc endLoc = castType.getSourceRange().makeEndSourceLoc();
			return ExprResult(
					new(ctxt_) CastExpr(
						castType.get(),
						prefixexpr.get(),
						begLoc,
						castType.getSourceRange(),
						endLoc
					)
				);
		}
		else
		{
			reportErrorExpected(DiagID::parser_expected_type);
			return ExprResult::Error();
		}
	}

	return prefixexpr;
}

Parser::ExprResult Parser::parseBinaryExpr(std::uint8_t precedence)
{
	// <binary_expr>  = <cast_expr> { <binary_operator> <cast_expr> }	

	// <cast_expr> OR a binaryExpr of inferior priority.
	ExprResult lhsResult;
	if (precedence > 0)
		lhsResult = parseBinaryExpr(precedence - 1);
	else
		lhsResult = parseCastExpr();

	if (!lhsResult)
	{
		if (!lhsResult.wasSuccessful())
			return ExprResult::Error();
		return ExprResult::NotFound();
	}

	Expr* lhs = lhsResult.get();
	BinaryExpr* rtr = nullptr;

	// { <binary_operator> <cast_expr> }	
	while (true)
	{
		// <binary_operator>
		auto binop_res = parseBinaryOp(precedence);
		if (!binop_res) // No operator found : break.
			break;

		// <cast_expr> OR a binaryExpr of inferior priority.
		ExprResult rhsResult;
		if (precedence > 0)
			rhsResult = parseBinaryExpr(precedence - 1);
		else
			rhsResult = parseCastExpr();


		// Handle results appropriately
		if (!rhsResult) // Check for validity : we need a rhs. if we don't have one, we have an error !
		{
			if(rhsResult.wasSuccessful())
				reportErrorExpected(DiagID::parser_expected_expr);
			return ExprResult::Error();
		}

		Expr* rhs = rhsResult.get();
		SourceLoc begLoc = lhs ? lhs->getBegLoc() : rtr->getEndLoc();
		SourceLoc endLoc = rhs->getEndLoc();
		SourceRange opRange = binop_res.getSourceRange();

		rtr = new(ctxt_) BinaryExpr(
				binop_res.get(), (rtr ? rtr : lhs), rhs, begLoc, opRange, endLoc
			);
	}

	if (!rtr)
	{
		assert(lhs && "no rtr node + no lhs node?");
		return ExprResult(lhs);
	}
	return ExprResult(rtr);
}

Parser::ExprResult Parser::parseExpr()
{
	//  <expr> = <binary_expr> [<assign_operator> <expr>] 
	auto lhs = parseBinaryExpr();
	if (!lhs)
		return lhs;

	if (auto op = parseAssignOp())
	{
		auto rhs = parseExpr();
		if (!rhs)
		{
			if(rhs.wasSuccessful())
				reportErrorExpected(DiagID::parser_expected_expr);
			return ExprResult::Error();
		}

		SourceLoc begLoc = lhs.get()->getBegLoc();
		SourceLoc endLoc = rhs.get()->getEndLoc();
		assert(begLoc && endLoc && "invalid locs");
		SourceRange opRange = op.getSourceRange();
		return ExprResult(
				new(ctxt_) BinaryExpr(op.get(), lhs.get(), rhs.get(), begLoc, opRange, endLoc)
			);
	}
	return ExprResult(lhs);
}

Parser::ExprResult Parser::parseParensExpr(bool isMandatory, SourceLoc* leftPLoc, SourceLoc* rightPLoc)
{
	// <parens_expr> = '(' <expr> ')'
	// '('
	auto leftParens = consumeBracket(SignType::S_ROUND_OPEN);
	if (!leftParens)
	{
		if (isMandatory)
		{
			reportErrorExpected(DiagID::parser_expected_opening_roundbracket);
			return ExprResult::Error();
		}
		return ExprResult::NotFound();
	}

	Expr* rtr = nullptr;
		
	// <expr>
	if (auto expr = parseExpr())
		rtr = expr.get();
	else 
	{
		// no expr, handle error & attempt to recover if it's allowed. If recovery is successful, return "not found"
		if(expr.wasSuccessful())
			reportErrorExpected(DiagID::parser_expected_expr);

		if (resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
			return ExprResult::NotFound();
		else
			return ExprResult::Error();
	}

	assert(rtr && "The return value shouldn't be null!");

	// ')'
	auto rightParens = consumeBracket(SignType::S_ROUND_CLOSE);
	if (!rightParens)
	{
		// no ), handle error & attempt to recover 
		reportErrorExpected(DiagID::parser_expected_closing_roundbracket);

		if (!resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ false))
			return ExprResult::Error();
			
		// If we recovered successfuly, place the Sloc into rightParens
		rightParens = consumeBracket(SignType::S_ROUND_CLOSE);
	}

	if (leftPLoc)
		*leftPLoc = leftParens;

	if (rightPLoc)
		*rightPLoc = rightParens;

	return ExprResult(
		new(ctxt_) ParensExpr(rtr, leftParens, rightParens)
	);
}

Parser::Result<ExprVector> Parser::parseExprList()
{
	// <expr_list> = <expr> {',' <expr> }
	auto firstexpr = parseExpr();
	if (!firstexpr)
		return Result<ExprVector>::NotFound();

	ExprVector exprs;
	exprs.push_back(firstexpr.get());
	while (auto comma = consumeSign(SignType::S_COMMA))
	{
		if (auto expr = parseExpr())
			exprs.push_back(expr.get());
		else
		{
			if (expr.wasSuccessful())
			{
				// if the expression was just not found, revert the comma consuming and
				// let the caller deal with the extra comma after the expression list.
				revertConsume();
				break;
			}

			return Result<ExprVector>::Error();
		}
	}

	return Result<ExprVector>(exprs);
}

Parser::Result<ExprVector> Parser::parseParensExprList(SourceLoc* LParenLoc, SourceLoc *RParenLoc)
{
	// <parens_expr_list>	= '(' [ <expr_list> ] ')'
	// '('
	auto leftParens = consumeBracket(SignType::S_ROUND_OPEN);
	if (!leftParens)
		return Result<ExprVector>::NotFound();

	if (LParenLoc)
		*LParenLoc = leftParens;

	ExprVector exprs;

	//  [ <expr_list> ]
	if (auto exprlist = parseExprList())
		exprs = exprlist.get();
	else if (!exprlist.wasSuccessful())
	{
		// error? Try to recover from it, if success, just discard the expr list,
		// if no success return error.
		if (resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ false))
		{
			SourceLoc loc = consumeBracket(SignType::S_ROUND_CLOSE);

			if (RParenLoc)
				*RParenLoc = loc;

			return Result<ExprVector>(ExprVector()); // if recovery is successful, return an empty expression list.
		}
		return Result<ExprVector>::Error();
	}

	SourceLoc rightParens = consumeBracket(SignType::S_ROUND_CLOSE);
	// ')'
	if (!rightParens)
	{
		reportErrorExpected(DiagID::parser_expected_closing_roundbracket);

		if (resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ false))
			rightParens = consumeBracket(SignType::S_ROUND_CLOSE);
		else 
			return Result<ExprVector>::Error();
	}

	if (RParenLoc)
		*RParenLoc = rightParens;

	return Result<ExprVector>(exprs);
}

SourceRange Parser::parseExponentOp()
{
	if (auto t1 = consumeSign(SignType::S_ASTERISK))
	{
		if (auto t2 = consumeSign(SignType::S_ASTERISK))
			return SourceRange(t1,t2);
		revertConsume();
	}
	return SourceRange();
}

Parser::Result<BinaryOperator> Parser::parseAssignOp()
{
	auto backup = createParserStateBackup();
	if (auto equal = consumeSign(SignType::S_EQUAL))
	{
		// Try to match a S_EQUAL. If failed, that means that the next token isn't a =
		// If it succeeds, we founda '==', this is the comparison operator and we must backtrack to prevent errors.
		if (!consumeSign(SignType::S_EQUAL))
			return Result<BinaryOperator>(BinaryOperator::ASSIGN_BASIC,SourceRange(equal));
		restoreParserStateFromBackup(backup);
	}
	return Result<BinaryOperator>::NotFound();
}

Parser::Result<UnaryOperator> Parser::parseUnaryOp()
{
	if (auto excl = consumeSign(SignType::S_EXCL_MARK))
		return Result<UnaryOperator>(UnaryOperator::LOGICNOT, SourceRange(excl));
	else if (auto minus = consumeSign(SignType::S_MINUS))
		return Result<UnaryOperator>(UnaryOperator::NEGATIVE, SourceRange(minus));
	else if (auto plus = consumeSign(SignType::S_PLUS))
		return Result<UnaryOperator>(UnaryOperator::POSITIVE, SourceRange(plus));
	return Result<UnaryOperator>::NotFound();
}

Parser::Result<BinaryOperator> Parser::parseBinaryOp(std::uint8_t priority)
{
	// Check current Token validity, also check if it's a sign because if it isn't we can return directly!
	if (!getCurtok().isValid() || !getCurtok().isSign())
		return Result<BinaryOperator>::NotFound();

	auto backup = createParserStateBackup();

	switch (priority)
	{
		case 0: // * / %
			if (auto asterisk = consumeSign(SignType::S_ASTERISK))
			{
				if (!consumeSign(SignType::S_ASTERISK)) // Disambiguation between '**' and '*'
					return Result<BinaryOperator>(BinaryOperator::MUL, SourceRange(asterisk));
			}
			else if (auto slash = consumeSign(SignType::S_SLASH))
				return Result<BinaryOperator>(BinaryOperator::DIV, SourceRange(slash));
			else if (auto percent = consumeSign(SignType::S_PERCENT))
				return Result<BinaryOperator>(BinaryOperator::MOD, SourceRange(percent));
			break;
		case 1: // + -
			if (auto plus = consumeSign(SignType::S_PLUS))
				return Result<BinaryOperator>(BinaryOperator::ADD, SourceRange(plus));
			else if (auto minus = consumeSign(SignType::S_MINUS))
				return Result<BinaryOperator>(BinaryOperator::MINUS, SourceRange(minus));
			break;
		case 2: // > >= < <=
			if (auto lessthan = consumeSign(SignType::S_LESS_THAN))
			{
				if (auto equal = consumeSign(SignType::S_EQUAL))
					return Result<BinaryOperator>(BinaryOperator::LESS_OR_EQUAL, SourceRange(lessthan,equal));
				return Result<BinaryOperator>(BinaryOperator::LESS_THAN, SourceRange(lessthan));
			}
			else if (auto grthan = consumeSign(SignType::S_GREATER_THAN))
			{
				if (auto equal = consumeSign(SignType::S_EQUAL))
					return Result<BinaryOperator>(BinaryOperator::GREATER_OR_EQUAL, SourceRange(grthan,equal));
				return Result<BinaryOperator>(BinaryOperator::GREATER_THAN, SourceRange(grthan));
			}
			break;
		case 3:	// == !=
			// try to match '=' twice.
			if (auto equal1 = consumeSign(SignType::S_EQUAL))
			{
				if (auto equal2 = consumeSign(SignType::S_EQUAL))
					return Result<BinaryOperator>(BinaryOperator::EQUAL, SourceRange(equal1,equal2));
			}
			else if (auto excl = consumeSign(SignType::S_EXCL_MARK))
			{
				if (auto equal =consumeSign(SignType::S_EQUAL))
					return Result<BinaryOperator>(BinaryOperator::NOTEQUAL, SourceRange(excl,equal));
			}
			break;
		case 4: // &&
			if (auto amp1 = consumeSign(SignType::S_AMPERSAND))
			{
				if (auto amp2 = consumeSign(SignType::S_AMPERSAND))
					return Result<BinaryOperator>(BinaryOperator::LOGIC_AND, SourceRange(amp1,amp2));
			}
			break;
		case 5: // ||
			if (auto vbar1 = consumeSign(SignType::S_VBAR))
			{
				if (auto vbar2 = consumeSign(SignType::S_VBAR))
					return Result<BinaryOperator>(BinaryOperator::LOGIC_OR, SourceRange(vbar1,vbar2));
			}
			break;
		default:
			fox_unreachable("Unknown priority");
			break;
	}
	restoreParserStateFromBackup(backup);; // Backtrack if we didn't return.
	return Result<BinaryOperator>::NotFound();
}