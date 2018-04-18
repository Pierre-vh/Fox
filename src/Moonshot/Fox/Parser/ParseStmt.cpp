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

ParsingResult<ASTCompoundStmt*> Parser::parseCompoundStatement(const bool& isMandatory)
{
	auto rtr = std::make_unique<ASTCompoundStmt>(); // return value
	if (matchSign(SignType::S_CURLY_OPEN))
	{
		// Parse all statements
		while (auto parseres = parseStmt())
		{
			if (!rtr->isEmpty())
			{
				// Don't push another null statement if the last statement is already a null one, to avoid stacking them up.
				if (dynamic_cast<ASTNullStmt*>(rtr->getBack()) &&
					dynamic_cast<ASTNullStmt*>(parseres.result.get()))
				{
					parseres = parseStmt();
					continue;
				}
			}
			rtr->addStmt(std::move(parseres.result));
			parseres = parseStmt();
		}
		// Match the closing curly bracket
		if (!matchSign(SignType::S_CURLY_CLOSE))
		{
			errorExpected("Expected a '}'");
			// try to recover, if recovery wasn't successful, report an error.
			if (!resyncToSign(SignType::S_CURLY_CLOSE))
				return ParsingResult<ASTCompoundStmt*>(false);
		}

		// if everything's alright, return the result
		return ParsingResult<ASTCompoundStmt*>(std::move(rtr));
	}
	// not found & mandatory
	else if (isMandatory)
	{
		// Emit an error if it's mandatory.
		errorExpected("Expected a '{'");
		return ParsingResult<ASTCompoundStmt*>(false);
	}

	return ParsingResult<ASTCompoundStmt*>();
}

ParsingResult<ASTStmt*> Parser::parseWhileLoop()
{
	// <while_loop>  = "while"	<parens_expr> <body>
	// "while"
	if (matchKeyword(KeywordType::KW_WHILE))
	{
		std::unique_ptr<ASTWhileStmt> rtr = std::make_unique<ASTWhileStmt>();
		// <parens_expr>
		if (auto parensExprRes = parseParensExpr(/* The ParensExpr is mandatory */ true)) 
			rtr->setCond(std::move(parensExprRes.result));

		// <body>
		if (auto parseres = parseBody())
			rtr->setBody(std::move(parseres.result));
		else
		{
			errorExpected("Expected a Statement after while loop declaration,");
			return ParsingResult<ASTStmt*>(false);
		}
		// Return if everything's alright
		return ParsingResult<ASTStmt*>(std::move(rtr));
	}
	// not found
	return ParsingResult<ASTStmt*>();
}

ParsingResult<ASTStmt*> Parser::parseCondition()
{
	// <condition>	= "if"	<parens_expr> <body> ["else" <body>]
	auto rtr = std::make_unique<ASTCondStmt>();
	bool has_if = false;
	// "if"
	if (matchKeyword(KeywordType::KW_IF))
	{
		// <parens_expr>
		if (auto parensExprRes = parseParensExpr(/* The ParensExpr is mandatory */ true)) 
			rtr->setCond(std::move(parensExprRes.result));

		// <body>
		if (auto ifStmtRes = parseBody())
			rtr->setThen(std::move(ifStmtRes.result));
		else
		{
			errorExpected("Expected a statement after if condition,");
			return ParsingResult<ASTStmt*>(false);
		}
		has_if = true;
	}
	// "else"
	if (matchKeyword(KeywordType::KW_ELSE))
	{
		// <body>
		if (auto stmt = parseBody())
			rtr->setElse(std::move(stmt.result));
		else
		{
			errorExpected("Expected a statement after else,");
			return ParsingResult<ASTStmt*>(false);
		}
		// Else but no if?
		if (!has_if)
			genericError("Else without matching if ignored.");
	}

	if(has_if)
		return ParsingResult<ASTStmt*>(std::move(rtr)); // success
	return ParsingResult<ASTStmt*>(); // not found
}

ParsingResult<ASTStmt*> Parser::parseReturnStmt()
{
	// <rtr_stmt> = "return" [<expr>] ';'
	// "return"
	if (matchKeyword(KeywordType::KW_RETURN))
	{
		auto rtr = std::make_unique<ASTReturnStmt>();
		// [<expr>]
		if (auto pExpr_res = parseExpr())
			rtr->setExpr(std::move(pExpr_res.result));

		if (!matchSign(SignType::S_SEMICOLON))
		{
			errorExpected("Expected a ';'");
			// Recover to semi, if recovery wasn't successful, report an error.
			if (!resyncToSign(SignType::S_SEMICOLON))
				return ParsingResult<ASTStmt*>(false); // failed & died
		}
		// success, return
		return ParsingResult<ASTStmt*>(std::move(rtr));
	}
	// not found
	return ParsingResult<ASTStmt*>();
}

ParsingResult<ASTStmt*> Parser::parseStmt()
{
	// <stmt>	= <var_decl> | <expr_stmt> | <condition> | <while_loop> | <rtr_stmt> 

	// Here, the code is always the same:
	// if result is usable
	//		return result
	// else if result was not successful (the parsing function recognized the rule, but an error occured while parsing it)
	//		return error

	// <var_decl
	if (auto vardecl = parseVarDeclStmt())
		return ParsingResult<ASTStmt*>(std::move(vardecl.result));
	else if (!vardecl.wasSuccessful())
		return ParsingResult<ASTStmt*>(false);

	// <expr_stmt>
	if (auto exprstmt = parseExprStmt())
		return exprstmt;
	else if (!exprstmt.wasSuccessful())
		return ParsingResult<ASTStmt*>(false);

	// <condition>
	if(auto cond = parseCondition())
		return cond;
	else if (!cond.wasSuccessful())
		return ParsingResult<ASTStmt*>(false);

	// <while_loop>
	if (auto wloop = parseWhileLoop())
		return wloop;
	else if(!wloop.wasSuccessful())
		return ParsingResult<ASTStmt*>(false);

	// <return_stmt>
	if (auto rtrstmt = parseReturnStmt())
		return rtrstmt;
	else if(!rtrstmt.wasSuccessful())
		return ParsingResult<ASTStmt*>(false);

	// Else, not found..
	return ParsingResult<ASTStmt*>();
}

ParsingResult<ASTStmt*> Parser::parseBody()
{
	// <body>	= <stmt> | <compound_statement>

	// <stmt>
	if (auto stmt_res = parseStmt())
		return stmt_res;
	else if(!stmt_res.wasSuccessful())
		return ParsingResult<ASTStmt*>(false);

	// <compound_statement>
	if (auto compstmt_res = parseCompoundStatement())
		return ParsingResult<ASTStmt*>(std::move(compstmt_res.result));
	else if(!compstmt_res.wasSuccessful())
		return ParsingResult<ASTStmt*>(false);

	return ParsingResult<ASTStmt*>();
}

ParsingResult<ASTStmt*> Parser::parseExprStmt()
{
	// <expr_stmt>	= ';' | <expr> ';' 

	// ';'
	if (matchSign(SignType::S_SEMICOLON))
		return ParsingResult<ASTStmt*>( std::make_unique<ASTNullStmt>() );

	// <expr> ';' 
	// <expr>
	if (auto node = parseExpr())
	{
		// ';'
		if (!matchSign(SignType::S_SEMICOLON))
		{
			errorExpected("Expected a ';' in expression statement");
			// attempt recovery, return error if couldn't recover.
			if (!resyncToSign(SignType::S_SEMICOLON))
				return ParsingResult<ASTStmt*>(false);
			// if recovery was successful, just return like nothing has happened!
		}

		return ParsingResult<ASTStmt*>(std::move(node.result));
	}
	else if(!node.wasSuccessful())
		return ParsingResult<ASTStmt*>(false);

	return ParsingResult<ASTStmt*>();
}