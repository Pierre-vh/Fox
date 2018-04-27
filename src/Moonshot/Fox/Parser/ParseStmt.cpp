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
	if (consumeBracket(SignType::S_CURLY_OPEN))
	{
		bool hasMatchedCurlyClose = false;
		while (!isDone())
		{
			if (consumeBracket(SignType::S_CURLY_CLOSE))
			{
				hasMatchedCurlyClose = true;
				break;
			}

			// try to parse a statement

			// success
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
					if (consumeBracket(SignType::S_CURLY_CLOSE))
					{
						hasMatchedCurlyClose = true;
						break;
					}
					// The second is that it found EOF or something that doesn't belong to us, like a unmatched ']' or ')'
					else
					{
						// (in this case, just return an error)
						return StmtResult::Error();
					}

				}
			}
		}
		// Match the closing curly bracket
		if (!hasMatchedCurlyClose)
		{
			errorExpected("Expected a '}'");
			// We can't recover since we reached EOF. return an error!
			return StmtResult::Error();
		}

		// if everything's alright, return the result
		return StmtResult(std::move(rtr));
	}
	// not found & mandatory
	else if (isMandatory)
	{
		// Emit an error if it's mandatory.
		errorExpected("Expected a '{'");

		// Here, we'll attempt to recover to a } and return an empty compound statement if we found one,
		// if we can't recover, we restore the backup and return "not found"
		// It's unlikely that one would forget both {}, but who knows!
		auto backup = createParserStateBackup();

		if (resyncToSign(SignType::S_CURLY_CLOSE, /* stopAtSemi */ false, /*consumeToken*/ true))
			return StmtResult(std::make_unique<CompoundStmt>());
		
		restoreParserStateFromBackup(backup);
	}
	return StmtResult::NotFound();
}

Parser::StmtResult Parser::parseWhileLoop()
{
	// <while_loop>  = "while"	<parens_expr> <body>
	// "while"
	if (consumeKeyword(KeywordType::KW_WHILE))
	{
		std::unique_ptr<WhileStmt> rtr = std::make_unique<WhileStmt>();
		// <parens_expr>
		if (auto parensexpr = parseParensExpr(/* The ParensExpr is mandatory */ true))
			rtr->setCond(parensexpr.move());
		else
			return StmtResult::Error();

		// <body>
		if (auto body = parseBody())
			rtr->setBody(body.move());
		else
		{
			if(body.wasSuccessful())
				errorExpected("Expected a statement");
			rtr->setBody(std::make_unique<NullExpr>());
		}
		// Return if everything's alright
		return StmtResult(std::move(rtr));
	}
	// not found
	return StmtResult::NotFound();
}

Parser::StmtResult Parser::parseCondition()
{
	// <condition>	= "if"	<parens_expr> <body> ["else" <body>]
	auto rtr = std::make_unique<ConditionStmt>();
	bool has_if = false;
	// "if"
	if (consumeKeyword(KeywordType::KW_IF))
	{
		// <parens_expr>
		if (auto parensexpr = parseParensExpr(/* The ParensExpr is mandatory */ true))
			rtr->setCond(parensexpr.move());
		else
			return StmtResult::Error();
		
		// <body>
		if (auto body = parseBody())
			rtr->setThen(body.move());
		else
		{
			if (body.wasSuccessful())
				errorExpected("Expected a statement after if condition,");

			if (consumeKeyword(KeywordType::KW_ELSE)) // if the user wrote something like if (<expr>) else, we'll recover by inserting a nullstmt
			{
				revertConsume();
				rtr->setThen(std::make_unique<NullExpr>());
			}
			else
				return StmtResult::Error();
		}
		has_if = true;
	}
	// "else"
	if (consumeKeyword(KeywordType::KW_ELSE))
	{
		// <body>
		if (auto stmt = parseBody())
			rtr->setElse(stmt.move());
		else
		{
			if(stmt.wasSuccessful())
				errorExpected("Expected a statement after else,");
			return StmtResult::Error();
		}

		// Else but no if?
		if (!has_if)
			genericError("Else without matching if.");
	}

	if (has_if)
		return StmtResult(std::move(rtr));

	return StmtResult::NotFound();
}

Parser::StmtResult Parser::parseReturnStmt()
{
	// <rtr_stmt> = "return" [<expr>] ';'
	// "return"
	if (consumeKeyword(KeywordType::KW_RETURN))
	{
		auto rtr = std::make_unique<ReturnStmt>();
		// [<expr>]
		if (auto expr = parseExpr())
			rtr->setExpr(expr.move());
		else if(!expr.wasSuccessful())
		{
			// expr failed? try to resync if possible. 
			if (!resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi */ false, /*consumeToken*/ true))
				return StmtResult::Error();
		}

		// ';'

		if (!consumeSign(SignType::S_SEMICOLON))
		{
			errorExpected("Expected a ';'");
			// Recover to semi, if recovery wasn't successful, report an error.
			if (!resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi */ false, /*consumeToken*/ true))
				return StmtResult::Error();
		}

		return StmtResult(std::move(rtr));
	}
	return StmtResult::NotFound();
}

Parser::StmtResult Parser::parseStmt()
{
	// <stmt>	= <var_decl> | <expr_stmt> | <condition> | <while_loop> | <rtr_stmt> 

	// Here, the code is always the same:
	// if result is usable
	//		return result
	// else if result was not successful (the parsing function recognized the rule, but an error occured while parsing it)
	//		return error

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
	if (consumeSign(SignType::S_SEMICOLON))
		return StmtResult(std::make_unique<NullExpr>());

	// <expr> ';' 
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