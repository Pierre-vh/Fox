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
				// Don't push another null statement if the last statement is already a null one.
				if (dynamic_cast<ASTNullStmt*>(rtr->getBack()) &&
					dynamic_cast<ASTNullStmt*>(parseres.result_.get()))
					continue;
			}
			rtr->addStmt(std::move(parseres.result_));
		}
		// Match the closing curly bracket
		if (!matchSign(SignType::S_CURLY_CLOSE))
		{
			errorExpected("Expected a closing curly bracket '}' at the end of the compound statement,");
			if (resyncToDelimiter(SignType::S_CURLY_CLOSE))
				return ParsingResult<ASTCompoundStmt*>(ParsingOutcome::FAILED_BUT_RECOVERED);
			return ParsingResult<ASTCompoundStmt*>(ParsingOutcome::FAILED_AND_DIED);
		}
		return ParsingResult<ASTCompoundStmt*>(ParsingOutcome::SUCCESS,std::move(rtr));
	}
	
	if (isMandatory)
	{
		errorExpected("Expected a '{'");
		if (resyncToDelimiter(SignType::S_CURLY_CLOSE))
			return ParsingResult<ASTCompoundStmt*>(ParsingOutcome::FAILED_BUT_RECOVERED);
		return ParsingResult<ASTCompoundStmt*>(ParsingOutcome::FAILED_AND_DIED);
	}
	return ParsingResult<ASTCompoundStmt*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTStmt*> Parser::parseWhileLoop()
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
			return ParsingResult<IASTStmt*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
		// Return
		return ParsingResult<IASTStmt*>(ps, std::move(rtr));
	}
	return ParsingResult<IASTStmt*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTStmt*> Parser::parseCondition()
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
		// no need for else since it manages error message in "mandatory" mode

		// <body>
		auto ifStmtRes = parseBody();
		if (ifStmtRes)
			rtr->setThen(std::move(ifStmtRes.result_));
		else
		{
			errorExpected("Expected a statement after if condition,");
			return ParsingResult<IASTStmt*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
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
			return ParsingResult<IASTStmt*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
		if (!has_if)
			genericError("Else without matching if.");
	}
	if(has_if)
		return ParsingResult<IASTStmt*>(ParsingOutcome::SUCCESS, std::move(rtr));
	return ParsingResult<IASTStmt*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTStmt*> Parser::parseReturnStmt()
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
			if (!resyncToDelimiter(SignType::S_SEMICOLON))
				return ParsingResult<IASTStmt*>(ParsingOutcome::FAILED_AND_DIED);
		}

		return ParsingResult<IASTStmt*>(ParsingOutcome::SUCCESS, std::move(rtr));
	}
	return ParsingResult<IASTStmt*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTStmt*> Parser::parseStmt()
{
	// <stmt>	= <var_decl> | <expr_stmt> | <condition> | <while_loop> | | <rtr_stmt> 
	if (auto parseres = parseExprStmt())
		return parseres;
	else if (auto parseres = parseVarDeclStmt())
		return parseres;
	else if (auto parseres = parseCondition())
		return parseres;
	else if (auto parseres = parseWhileLoop())
		return parseres;
	
	else if (auto parseres = parseReturnStmt())
		return parseres;
	else
		return ParsingResult<IASTStmt*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTStmt*> Parser::parseBody()
{
	if (auto parseres = parseStmt())
		return parseres;
	else if (auto parseres = parseCompoundStatement())
		return ParsingResult<IASTStmt*>(parseres.getFlag(), std::move(parseres.result_));
	return ParsingResult<IASTStmt*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTStmt*> Parser::parseExprStmt()
{
	//<expr_stmt> = ';' |<expr> ';'
	if (matchSign(SignType::S_SEMICOLON))
		return ParsingResult<IASTStmt*>(ParsingOutcome::SUCCESS,
			std::make_unique<ASTNullStmt>()
		);
	else if (auto node = parseExpr())
	{
		// Found node
		if (matchSign(SignType::S_SEMICOLON))
			return ParsingResult<IASTStmt*>(ParsingOutcome::SUCCESS,std::move(node.result_));
		else
		{
			errorExpected("Expected a ';' in expression statement");
			if (resyncToDelimiter(SignType::S_SEMICOLON))
				return ParsingResult<IASTStmt*>(ParsingOutcome::FAILED_BUT_RECOVERED);
			return ParsingResult<IASTStmt*>(ParsingOutcome::FAILED_AND_DIED);
		}
	}
	return ParsingResult<IASTStmt*>(ParsingOutcome::NOTFOUND);
}