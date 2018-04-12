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
	auto compstmt = parseTopLevelCompoundStatement(isMandatory);
	if (compstmt.getFlag() == ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY)
	{
		if (resyncToSign(SignType::S_CURLY_CLOSE))
		{
			if (compstmt.isDataAvailable())
				return ParsingResult<ASTCompoundStmt*>(ParsingOutcome::FAILED_BUT_RECOVERED, std::move(compstmt.result_));
			return ParsingResult<ASTCompoundStmt*>(ParsingOutcome::FAILED_BUT_RECOVERED);
		}
		return ParsingResult<ASTCompoundStmt*>(ParsingOutcome::FAILED_AND_DIED);
	}
	return compstmt;
}

ParsingResult<ASTCompoundStmt*> Parser::parseTopLevelCompoundStatement(const bool& isMandatory)
{
	auto rtr = std::make_unique<ASTCompoundStmt>(); // return value
	if (matchSign(SignType::S_CURLY_OPEN))
	{
		// Parse all statements
		auto parseres = parseStmt();
		while (parseres.isDataAvailable())
		{
			if (!rtr->isEmpty())
			{
				// Don't push another null statement if the last statement is already a null one, to avoid stacking them up.
				if (dynamic_cast<ASTNullStmt*>(rtr->getBack()) &&
					dynamic_cast<ASTNullStmt*>(parseres.result_.get()))
				{
					parseres = parseStmt();
					continue;
				}
			}
			rtr->addStmt(std::move(parseres.result_));
			parseres = parseStmt();
		}
		// Match the closing curly bracket
		if (!matchSign(SignType::S_CURLY_CLOSE))
		{
			errorExpected("Expected a closing curly bracket '}' at the end of the compound statement,");
			return ParsingResult<ASTCompoundStmt*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY,std::move(rtr));
		}
		return ParsingResult<ASTCompoundStmt*>(ParsingOutcome::SUCCESS,std::move(rtr));
	}
	else if (isMandatory)
		errorExpected("Expected a '{'");

	return ParsingResult<ASTCompoundStmt*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<ASTStmt*> Parser::parseWhileLoop()
{
	// Rule : <while_loop> 	= <wh_kw>  '(' <expr> ')' <body> 
	if (matchKeyword(KeywordType::KW_WHILE))
	{
		ParsingOutcome ps = ParsingOutcome::SUCCESS;
		std::unique_ptr<ASTWhileStmt> rtr = std::make_unique<ASTWhileStmt>();
		// <parens_expr>
		if (auto parensExprRes = parseParensExpr(true,true)) // true -> parensExpr is mandatory.
			rtr->setCond(std::move(parensExprRes.result_));
		else
			ps = parensExprRes.getFlag();
		// <body>
		if (auto parseres = parseBody())
			rtr->setBody(std::move(parseres.result_));
		else
		{
			errorExpected("Expected a Statement after while loop declaration,");
			return ParsingResult<ASTStmt*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
		// Return
		return ParsingResult<ASTStmt*>(ps, std::move(rtr));
	}
	return ParsingResult<ASTStmt*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<ASTStmt*> Parser::parseCondition()
{
	//<condition> = "if" <parens_expr> <body> ["else" <statement>]
	auto rtr = std::make_unique<ASTCondStmt>();
	bool has_if = false;
	// "if"
	if (matchKeyword(KeywordType::KW_IF))
	{
		// <parens_expr>
		if (auto parensExprRes = parseParensExpr(true,true)) // true -> parensExpr is mandatory.
			rtr->setCond(std::move(parensExprRes.result_));
		// no need for else since it parseParensExpr error message in "mandatory" mode

		// <body>
		auto ifStmtRes = parseBody();
		if (ifStmtRes)
			rtr->setThen(std::move(ifStmtRes.result_));
		else
		{
			errorExpected("Expected a statement after if condition,");
			return ParsingResult<ASTStmt*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
		has_if = true;
	}
	// "else"
	if (matchKeyword(KeywordType::KW_ELSE))
	{
		// <body>
		if (auto stmt = parseBody())
			rtr->setElse(std::move(stmt.result_));
		else
		{
			errorExpected("Expected a statement after else,");
			return ParsingResult<ASTStmt*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
		if (!has_if)
			genericError("Else without matching if.");
	}
	if(has_if)
		return ParsingResult<ASTStmt*>(ParsingOutcome::SUCCESS, std::move(rtr));
	return ParsingResult<ASTStmt*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<ASTStmt*> Parser::parseReturnStmt()
{
	// <rtr_stmt>	= "return" [<expr>] ';'
	if (matchKeyword(KeywordType::KW_RETURN))
	{
		auto rtr = std::make_unique<ASTReturnStmt>();
		if (auto pExpr_res = parseExpr())
			rtr->setExpr(std::move(pExpr_res.result_));

		if (!matchSign(SignType::S_SEMICOLON))
		{
			errorExpected("Expected a ';'");
			if (!resyncToSign(SignType::S_SEMICOLON))
				return ParsingResult<ASTStmt*>(ParsingOutcome::FAILED_AND_DIED);
		}

		return ParsingResult<ASTStmt*>(ParsingOutcome::SUCCESS, std::move(rtr));
	}
	return ParsingResult<ASTStmt*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<ASTStmt*> Parser::parseStmt()
{
	// <stmt>	= <var_decl> | <expr_stmt> | <condition> | <while_loop> | | <rtr_stmt> 
	// <expr_stmt>
	auto exprstmt = parseExprStmt();
	if (exprstmt.getFlag() != ParsingOutcome::NOTFOUND)
		return exprstmt;
	// <var_decl
	auto vardecl = parseVarDeclStmt();
	if (vardecl.getFlag() != ParsingOutcome::NOTFOUND)
	{
		if (vardecl.isDataAvailable())
			return ParsingResult<ASTStmt*>(vardecl.getFlag(), std::move(vardecl.result_));
		return ParsingResult<ASTStmt*>(vardecl.getFlag());
	}
	// <condition>
	auto cond = parseCondition();
	if(cond.getFlag() != ParsingOutcome::NOTFOUND)
		return cond;
	// <while_loop>
	auto wloop = parseWhileLoop();
	if (wloop.getFlag() != ParsingOutcome::NOTFOUND)
		return wloop;
	// <return_stmt>
	auto rtrstmt = parseReturnStmt();
	if (rtrstmt.getFlag() != ParsingOutcome::NOTFOUND)
		return rtrstmt;
	// Else, not found..
	return ParsingResult<ASTStmt*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<ASTStmt*> Parser::parseBody()
{
	if (auto stmt_res = parseStmt())
		return stmt_res;
	else if (auto compstmt_res = parseCompoundStatement())
		return ParsingResult<ASTStmt*>(compstmt_res.getFlag(), std::move(compstmt_res.result_));
	return ParsingResult<ASTStmt*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<ASTStmt*> Parser::parseExprStmt()
{
	//<expr_stmt> = ';' |<expr> ';'
	if (matchSign(SignType::S_SEMICOLON))
		return ParsingResult<ASTStmt*>(ParsingOutcome::SUCCESS,
			std::make_unique<ASTNullStmt>()
		);
	else if (auto node = parseExpr())
	{
		// Found node
		if (matchSign(SignType::S_SEMICOLON))
			return ParsingResult<ASTStmt*>(ParsingOutcome::SUCCESS,std::move(node.result_));
		else
		{
			errorExpected("Expected a ';' in expression statement");
			if (resyncToSign(SignType::S_SEMICOLON))
				return ParsingResult<ASTStmt*>(ParsingOutcome::FAILED_BUT_RECOVERED);
			return ParsingResult<ASTStmt*>(ParsingOutcome::FAILED_AND_DIED);
		}
	}
	return ParsingResult<ASTStmt*>(ParsingOutcome::NOTFOUND);
}