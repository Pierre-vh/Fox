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
#include "Moonshot/Fox/AST/ASTStmt.hpp"

using namespace Moonshot;

ParseRes<ASTCompoundStmt*> Parser::parseCompoundStatement(const bool& isMandatory)
{
	auto rtr = std::make_unique<ASTCompoundStmt>(); // return value
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
			if(auto parseres = parseStmt())
			{
				// Push only if we don't have a null expr.
				// this is done to avoid stacking them up, and since they're a no-op in all cases
				// it's meaningless to ignore them.
				if (!(dynamic_cast<ASTNullExpr*>(parseres.result.get())))
					rtr->addStmt(std::move(parseres.result));
			}
			// failure
			else
			{
				// if not found, report an error
				if (parseres.wasSuccessful())
					errorExpected("Expected a Statement!");
				// In both case, attempt recovery to nearest semicolon.
				if (resyncToSign(SignType::S_SEMICOLON,/*stopAtSemi -> meaningless here*/ false, /*shouldConsumeToken*/ true))
				{
					// Successfully resynced, continue parsing.
					continue;
				}
				else
				{
					// Recovery might have stopped for 3 reasons (in order of likelihood), the first is that it found a unmatched }, which is ours.
					if (consumeBracket(SignType::S_CURLY_CLOSE))
					{
						hasMatchedCurlyClose = true;
						break;
					}
					// The second is that it found EOF or something that doesn't belong to us, like a unmatched ']' or ')'
					else
					{
						// (in this case, just return an error)
						return ParseRes<ASTCompoundStmt*>(false);
					}

				}
			}
		}
		// Match the closing curly bracket
		if (!hasMatchedCurlyClose)
		{
			errorExpected("Expected a '}'");
			// We can't recover since we reached EOF. return an error!
			return ParseRes<ASTCompoundStmt*>(false);
		}

		// if everything's alright, return the result
		return ParseRes<ASTCompoundStmt*>(std::move(rtr));
	}
	// not found & mandatory
	else if (isMandatory)
	{
		// Emit an error if it's mandatory.
		errorExpected("Expected a '{'");

		// Here, we'll attempt to recover to a } and return an empty compound statement if we found one,
		// if we can't recover, we restore the backup and return "not found"
		auto backup = createParserStateBackup();

		if (resyncToSign(SignType::S_CURLY_CLOSE, /* stopAtSemi */ false, /*consumeToken*/ true))
			return ParseRes<ASTCompoundStmt*>(std::make_unique<ASTCompoundStmt>());
		
		restoreParserStateFromBackup(backup);
		return ParseRes<ASTCompoundStmt*>();
	}

	return ParseRes<ASTCompoundStmt*>();
}

ParseRes<ASTStmt*> Parser::parseWhileLoop()
{
	// <while_loop>  = "while"	<parens_expr> <body>
	// "while"
	if (consumeKeyword(KeywordType::KW_WHILE))
	{
		std::unique_ptr<ASTWhileStmt> rtr = std::make_unique<ASTWhileStmt>();
		// <parens_expr>
		if (auto parensExprRes = parseParensExpr(/* The ParensExpr is mandatory */ true))
			rtr->setCond(std::move(parensExprRes.result));
		else
			return ParseRes<ASTStmt*>(false);

		// <body>
		if (auto parseres = parseBody())
			rtr->setBody(std::move(parseres.result));
		else
		{
			if(parseres.wasSuccessful())
				errorExpected("Expected a statement");
			rtr->setBody(std::make_unique<ASTNullExpr>());
		}
		// Return if everything's alright
		return ParseRes<ASTStmt*>(std::move(rtr));
	}
	// not found
	return ParseRes<ASTStmt*>();
}

ParseRes<ASTStmt*> Parser::parseCondition()
{
	// <condition>	= "if"	<parens_expr> <body> ["else" <body>]
	auto rtr = std::make_unique<ASTCondStmt>();
	bool has_if = false;
	// "if"
	if (consumeKeyword(KeywordType::KW_IF))
	{
		// <parens_expr>
		if (auto parensExprRes = parseParensExpr(/* The ParensExpr is mandatory */ true)) 
			rtr->setCond(std::move(parensExprRes.result));
		else
			return ParseRes<ASTStmt*>(false);
		
		// <body>
		if (auto ifStmtRes = parseBody())
			rtr->setThen(std::move(ifStmtRes.result));
		else
		{
			auto tok = getCurtok();
			if (tok.isKeyword() && tok.getKeywordType() == KeywordType::KW_ELSE) // if the user wrote something like if (<expr>) else, we'll recover by inserting a nullstmt
				rtr->setThen(std::make_unique<ASTNullExpr>());
			else
			{
				if (ifStmtRes.wasSuccessful())
					errorExpected("Expected a statement after if condition,");
				return ParseRes<ASTStmt*>(false);
			}
		}
		has_if = true;
	}
	// "else"
	if (consumeKeyword(KeywordType::KW_ELSE))
	{
		// <body>
		if (auto stmt = parseBody())
			rtr->setElse(std::move(stmt.result));
		else
		{
			if(stmt.wasSuccessful())
				errorExpected("Expected a statement after else,");
			return ParseRes<ASTStmt*>(false);
		}

		// Else but no if?
		if (!has_if)
			genericError("Else without matching if.");
	}

	if(has_if)
		return ParseRes<ASTStmt*>(std::move(rtr)); // success
	return ParseRes<ASTStmt*>(); // not found
}

ParseRes<ASTStmt*> Parser::parseReturnStmt()
{
	// <rtr_stmt> = "return" [<expr>] ';'
	// "return"
	if (consumeKeyword(KeywordType::KW_RETURN))
	{
		auto rtr = std::make_unique<ASTReturnStmt>();
		// [<expr>]
		if (auto pExpr_res = parseExpr())
			rtr->setExpr(std::move(pExpr_res.result));
		else if(!pExpr_res.wasSuccessful())
		{
			// expr failed? try to resync if possible. 
			if (!resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi */ false, /*consumeToken*/ true))
				return ParseRes<ASTStmt*>(false);
		}

		// ';'

		if (!consumeSign(SignType::S_SEMICOLON))
		{
			errorExpected("Expected a ';'");
			// Recover to semi, if recovery wasn't successful, report an error.
			if (!resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi */ false, /*consumeToken*/ true))
				return ParseRes<ASTStmt*>(false);
		}
		// success, return
		return ParseRes<ASTStmt*>(std::move(rtr));
	}
	// not found
	return ParseRes<ASTStmt*>();
}

ParseRes<ASTStmt*> Parser::parseStmt()
{
	// <stmt>	= <var_decl> | <expr_stmt> | <condition> | <while_loop> | <rtr_stmt> 

	// Here, the code is always the same:
	// if result is usable
	//		return result
	// else if result was not successful (the parsing function recognized the rule, but an error occured while parsing it)
	//		return error

	// <var_decl
	if (auto vardecl = parseVarDeclStmt())
		return ParseRes<ASTStmt*>(std::move(vardecl.result));
	else if (!vardecl.wasSuccessful())
		return ParseRes<ASTStmt*>(false);

	// <expr_stmt>
	if (auto exprstmt = parseExprStmt())
		return exprstmt;
	else if (!exprstmt.wasSuccessful())
		return ParseRes<ASTStmt*>(false);

	// <condition>
	if(auto cond = parseCondition())
		return cond;
	else if (!cond.wasSuccessful())
		return ParseRes<ASTStmt*>(false);

	// <while_loop>
	if (auto wloop = parseWhileLoop())
		return wloop;
	else if(!wloop.wasSuccessful())
		return ParseRes<ASTStmt*>(false);

	// <return_stmt>
	if (auto rtrstmt = parseReturnStmt())
		return rtrstmt;
	else if(!rtrstmt.wasSuccessful())
		return ParseRes<ASTStmt*>(false);

	// Else, not found..
	return ParseRes<ASTStmt*>();
}

ParseRes<ASTStmt*> Parser::parseBody()
{
	// <body>	= <stmt> | <compound_statement>

	// <stmt>
	if (auto stmt_res = parseStmt())
		return stmt_res;
	else if(!stmt_res.wasSuccessful())
		return ParseRes<ASTStmt*>(false);

	// <compound_statement>
	if (auto compstmt_res = parseCompoundStatement())
		return ParseRes<ASTStmt*>(std::move(compstmt_res.result));
	else if(!compstmt_res.wasSuccessful())
		return ParseRes<ASTStmt*>(false);

	return ParseRes<ASTStmt*>();
}

ParseRes<ASTStmt*> Parser::parseExprStmt()
{
	// <expr_stmt>	= ';' | <expr> ';' 

	// ';'
	if (consumeSign(SignType::S_SEMICOLON))
		return ParseRes<ASTStmt*>( std::make_unique<ASTNullExpr>() );

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
				return ParseRes<ASTStmt*>(false);
			// if recovery was successful, just return like nothing has happened!
		}

		return ParseRes<ASTStmt*>(std::move(expr.result));
	}
	else if(!expr.wasSuccessful())
	{
		// if the expression had an error, ignore it and try to recover to a semi.
		if (resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi */ false, /*consumeToken*/ true))
			return ParseRes<ASTStmt*>(std::make_unique<ASTNullExpr>());
		return ParseRes<ASTStmt*>(false);
	}

	return ParseRes<ASTStmt*>();
}