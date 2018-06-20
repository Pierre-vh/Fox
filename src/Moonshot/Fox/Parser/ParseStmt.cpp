////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ParseStmt.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
//	This file implements statements rules. parseStmt, parseVarDeclstmt,etc.									
////------------------------------------------------------////

#include "Parser.hpp"
//Nodes
#include "Moonshot/Fox/AST/Stmt.hpp"

using namespace Moonshot;

Parser::StmtResult Parser::parseCompoundStatement(const bool& isMandatory)
{
	auto rtr = std::make_unique<CompoundStmt>(); // return value
	if (auto LeftCurlyLoc = consumeBracket(SignType::S_CURLY_OPEN))
	{
		SourceLoc RightCurlyLoc;
		while (!isDone())
		{
			if (RightCurlyLoc = consumeBracket(SignType::S_CURLY_CLOSE))
				break;

			// try to parse a statement
			if(auto stmt = parseStmt())
			{
				// Push only if we don't have a null expr.
				// this is done to avoid stacking them up, and since they're a no-op in all cases
				// it's meaningless to ignore them.
				if (!stmt.is<NullExpr>())
					rtr->addStmt(std::move(stmt.move()));
			}
			// failure
			else
			{
				// if not found, report an error
				if (stmt.wasSuccessful())
					errorExpected("Expected a Statement");
				// In both case, attempt recovery to nearest semicolon.
				if (resyncToSign(SignType::S_SEMICOLON,/*stopAtSemi -> meaningless here*/ false, /*shouldConsumeToken*/ true))
				{
					// Successfully resynced, continue parsing.
					continue;
				}
				else
				{
					// Recovery might have stopped for 2 reasons, the first is that it found a unmatched }, which is ours.
					if (RightCurlyLoc = consumeBracket(SignType::S_CURLY_CLOSE))
						break;
					// The second is that it found EOF or something that doesn't belong to us, like a unmatched ']' or ')'
					else
						return StmtResult::Error();	// (in this case, just return an error)
				}
			}
		}

		if (!RightCurlyLoc.isValid())
		{
			errorExpected("Expected a '}'");
			// We can't recover since we reached EOF. return an error!
			return StmtResult::Error();
		}

		// if everything's alright, return the result
		rtr->setSourceLocs(LeftCurlyLoc, RightCurlyLoc);
		assert(rtr->hasLocInfo() && "No loc info? But we just set it!");
		return StmtResult(std::move(rtr));
	}
	// not found & mandatory
	else if (isMandatory)
	{
		// Emit an error if it's mandatory.
		errorExpected("Expected a '{'");

		// Here, we'll attempt to recover to a } if possible (in case someone missed the { )
		// if we can't recover, we restore the backup and return "not found". 
		auto backup = createParserStateBackup();

		if (!resyncToSign(SignType::S_CURLY_CLOSE, /* stopAtSemi */ false, /*consumeToken*/ true))
			restoreParserStateFromBackup(backup);
	}
	return StmtResult::NotFound();
}

Parser::StmtResult Parser::parseWhileLoop()
{
	// <while_loop>  = "while"	<parens_expr> <body>
	// "while"
	if (auto whKw = consumeKeyword(KeywordType::KW_WHILE))
	{
		std::unique_ptr<Expr> expr;
		std::unique_ptr<Stmt> body;
		SourceLoc parenExprEndLoc;
		SourceLoc begLoc = whKw.getBeginSourceLoc();
		SourceLoc endLoc;
		// <parens_expr>
		if (auto parens_expr_res = parseParensExpr(/* The ParensExpr is mandatory */ true, nullptr, &parenExprEndLoc))
		{
			assert(parenExprEndLoc && "The ParensExpr Successfully, but didn't provide the ')' loc?");
			expr = parens_expr_res.move();
		}
		else
			return StmtResult::Error();

		// <body>
		if (auto body_res = parseBody())
		{
			body = body_res.move();
			endLoc = body->getEndLoc();
			assert(endLoc && "The body parsed successfully, but doesn't have a valid endLoc?");
		}
		else
		{
			if(body_res.wasSuccessful())
				errorExpected("Expected a statement");
			body = std::make_unique<NullExpr>();
			endLoc = parenExprEndLoc;
		}

		assert(expr && body && begLoc && endLoc && parenExprEndLoc);
		return StmtResult(std::make_unique<WhileStmt>(
				std::move(expr),
				std::move(body),
				begLoc,
				parenExprEndLoc,
				endLoc
			));
	}
	// not found
	return StmtResult::NotFound();
}

Parser::StmtResult Parser::parseCondition()
{
	// <condition>	= "if"	<parens_expr> <body> ["else" <body>]
	std::unique_ptr<Expr> expr;
	std::unique_ptr<Stmt> ifBody, elseBody;
	SourceLoc begLoc, ifHeadEndLoc, endLoc;
	// "if"
	if (auto ifKw = consumeKeyword(KeywordType::KW_IF))
	{
		begLoc = ifKw.getBeginSourceLoc();
		// <parens_expr>
		if (auto parensexpr = parseParensExpr(/* The ParensExpr is mandatory */ true, nullptr, &ifHeadEndLoc))
			expr = parensexpr.move();
		else
			return StmtResult::Error();
		
		// <body>
		if (auto body = parseBody())
			ifBody = body.move();
		else
		{
			if (body.wasSuccessful())
				errorExpected("Expected a statement after if condition,");

			if (peekNext(KeywordType::KW_ELSE)) // if the user wrote something like if (<expr>) else, we'll recover by inserting a nullstmt
				ifBody = std::make_unique<NullExpr>();
			else
				return StmtResult::Error();
		}

		// "else"
		if (consumeKeyword(KeywordType::KW_ELSE))
		{
			// <body>
			if (auto stmt = parseBody())
				elseBody = stmt.move();
			else
			{
				if(stmt.wasSuccessful())
					errorExpected("Expected a statement after else,");
				return StmtResult::Error();
			}

			// Else but no if?
			if (!ifBody)
				genericError("Else without matching if.");
		}

		return StmtResult(std::make_unique<ConditionStmt>(
				std::move(expr),
				std::move(ifBody),
				std::move(elseBody),
				begLoc, 
				ifHeadEndLoc,
				endLoc
			));
	}
	return StmtResult::NotFound();
}

Parser::StmtResult Parser::parseReturnStmt()
{
	// <rtr_stmt> = "return" [<expr>] ';'
	// "return"
	if (auto rtrKw = consumeKeyword(KeywordType::KW_RETURN))
	{
		std::unique_ptr<Expr> expr;
		SourceLoc begLoc = rtrKw.getBeginSourceLoc();
		SourceLoc endLoc;

		// [<expr>]
		if (auto expr_res = parseExpr())
			expr = expr_res.move();
		else if(!expr_res.wasSuccessful())
		{
			// expr failed? try to resync if possible. 
			if (!resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi */ false, /*consumeToken*/ true))
				return StmtResult::Error();
		}

		// ';'
		if (auto semi = consumeSign(SignType::S_SEMICOLON))
			endLoc = semi;
		else
		{
			errorExpected("Expected a ';'");
			// Recover to semi, if recovery wasn't successful, report an error.
			if (!resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi */ false, /*consumeToken*/ true))
				return StmtResult::Error();
		}
		
		assert(begLoc && endLoc);
		return StmtResult(std::make_unique<ReturnStmt>(
				std::move(expr),
				begLoc, 
				endLoc
			));
	}
	return StmtResult::NotFound();
}

Parser::StmtResult Parser::parseStmt()
{
	// <stmt>	= <var_decl> | <expr_stmt> | <condition> | <while_loop> | <rtr_stmt> 

	// <var_decl
	if (auto vardecl = parseVarDecl())
		return StmtResult(std::make_unique<DeclStmt>(vardecl.move()));
	else if (!vardecl.wasSuccessful())
		return StmtResult::Error();

	// <expr_stmt>
	if (auto exprstmt = parseExprStmt())
		return exprstmt;
	else if (!exprstmt.wasSuccessful())
		return StmtResult::Error();

	// <condition>
	if(auto cond = parseCondition())
		return cond;
	else if (!cond.wasSuccessful())
		return StmtResult::Error();

	// <while_loop>
	if (auto wloop = parseWhileLoop())
		return wloop;
	else if(!wloop.wasSuccessful())
		return StmtResult::Error();

	// <return_stmt>
	if (auto rtrstmt = parseReturnStmt())
		return rtrstmt;
	else if(!rtrstmt.wasSuccessful())
		return StmtResult::Error();

	return StmtResult::NotFound();
}

Parser::StmtResult Parser::parseBody()
{
	// <body>	= <stmt> | <compound_statement>

	// <stmt>
	if (auto stmt = parseStmt())
		return stmt;
	else if (!stmt.wasSuccessful())
		return StmtResult::Error();

	// <compound_statement>
	if (auto compoundstmt = parseCompoundStatement())
		return compoundstmt;
	else if (!compoundstmt.wasSuccessful())
		return StmtResult::Error();

	return StmtResult::NotFound();
}

Parser::StmtResult Parser::parseExprStmt()
{
	// <expr_stmt>	= ';' | <expr> ';' 	

	// ';'
	if (auto semiLoc = consumeSign(SignType::S_SEMICOLON))
		return StmtResult(std::make_unique<NullExpr>(semiLoc));

	// <expr> 
	else if (auto expr = parseExpr())
	{
		// ';'
		if (!consumeSign(SignType::S_SEMICOLON))
		{
			if(expr.wasSuccessful())
				errorExpected("Expected a ';'");

			if (!resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi */ false, /*consumeToken*/ true))
				return StmtResult::Error();
			// if recovery was successful, just return like nothing has happened!
		}

		return StmtResult(expr.move());
	}
	else if(!expr.wasSuccessful())
	{
		// if the expression had an error, ignore it and try to recover to a semi.
		if (resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi */ false, /*consumeToken*/ true))
			return StmtResult(std::make_unique<NullExpr>());
		return StmtResult::Error();
	}

	return StmtResult::NotFound();
}