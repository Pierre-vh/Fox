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

using namespace Moonshot;

using sign = Token::sign;
using keyword = Token::keyword;

// Context and Exceptions
#include "Moonshot/Common/Context/Context.hpp"
#include "Moonshot/Common/Exceptions/Exceptions.hpp"

//Nodes
#include "Moonshot/Fox/AST/Nodes/ASTCompStmt.hpp"
#include "Moonshot/Fox/AST/Nodes/IASTStmt.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTCondition.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTWhileLoop.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTVarDeclStmt.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTNullStmt.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTFunctionDeclaration.hpp"

ParsingResult<IASTStmt> Parser::parseCompoundStatement()
{
	auto rtr = std::make_unique<ASTCompStmt>(); // return value
	if (matchSign(sign::B_CURLY_OPEN))
	{
		// Parse all statements
		while (auto parseres = parseStmt())
		{
			if (rtr->statements_.size())
			{
				// Don't push another null statement if the last statement is already a null one.
				if (dynamic_cast<ASTNullStmt*>(rtr->statements_.back().get()) &&
					dynamic_cast<ASTNullStmt*>(parseres.node_.get()))
					continue;
			}
			rtr->statements_.push_back(std::move(parseres.node_));
		}
		// Match the closing curly bracket
		if (!matchSign(sign::B_CURLY_CLOSE))
		{
			errorExpected("Expected a closing curly bracket '}' at the end of the compound statement,");
			if (resyncToDelimiter(sign::B_CURLY_CLOSE))
				return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_BUT_RECOVERED);
			return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_AND_DIED);
		}
		return ParsingResult<IASTStmt>(ParsingOutcome::SUCCESS,std::move(rtr));
	}
	return ParsingResult<IASTStmt>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTStmt> Parser::parseWhileLoop()
{
	// Rule : <while_loop> 	= <wh_kw>  '(' <expr> ')'	<compound_statement> 
	if (matchKeyword(keyword::D_WHILE))
	{
		std::unique_ptr<ASTWhileLoop> rtr = std::make_unique<ASTWhileLoop>();
		// <parens_expr>
		if (auto parensExprRes = parseParensExpr(true)) // true -> parensExpr is mandatory.
			rtr->expr_ = std::move(parensExprRes.node_);
			//  no need for failure cases, the function parseParensExpr manages failures by itself when the mandatory flag is set.
		// <stmt>
		if (auto parseres = parseStmt())
			rtr->body_ = std::move(parseres.node_);
		else
		{
			errorExpected("Expected a Statement after while loop declaration,");
			return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
		// Return
		return ParsingResult<IASTStmt>(ParsingOutcome::SUCCESS, std::move(rtr));
	}
	return ParsingResult<IASTStmt>(ParsingOutcome::NOTFOUND);
}

ParsingResult<ASTFunctionDeclaration> Parser::parseFunctionDeclaration()
{
	/* 
		<func_decl> = "func" <id> '(' [<arg_list_decl>] ')'[':' <type>] <compound_statement>	// Note about type_spec : if it is not present, the function returns void.
		<arg_list_decl> = [<arg_decl> {',' <arg_decl>}*]
		<arg_decl> = <id> : ["const"]['&'] <type>
	*/
	return ParsingResult<ASTFunctionDeclaration>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTStmt> Parser::parseCondition()
{
	//<condition> = "if" <parens_expr> <statement> ["else" <statement>]
	auto rtr = std::make_unique<ASTCondition>();
	bool has_if = false;
	// "if"
	if (matchKeyword(keyword::D_IF))
	{
		// <parens_expr>
		if (auto parensExprRes = parseParensExpr(true)) // true -> parensExpr is mandatory.
			rtr->condition_expr_ = std::move(parensExprRes.node_);

		// <statement>
		auto ifStmtRes = parseStmt();
		if (ifStmtRes)
			rtr->condition_stmt_ = std::move(ifStmtRes.node_);
		else
		{
			errorExpected("Expected a statement after if condition,");
			return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
		has_if = true;
	}
	// "else"
	if (matchKeyword(keyword::D_ELSE))
	{
		// <statement>
		if (auto stmt = parseStmt())
			rtr->else_stmt_ = std::move(stmt.node_);
		else
		{
			errorExpected("Expected a statement after else,");
			return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
		if (!has_if)
			genericError("Else without matching if.");
	}
	if(has_if)
		return ParsingResult<IASTStmt>(ParsingOutcome::SUCCESS, std::move(rtr));
	return ParsingResult<IASTStmt>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTStmt> Parser::parseStmt()
{
	// <stmt>	= <var_decl> | <expr_stmt> | <condition> | <while_loop> | <compound_statement> | (<rtr_stmt> -> to be implemented)
	if (auto parseres = parseExprStmt())
		return parseres;
	else if (auto parseres = parseVarDeclStmt())
		return parseres;
	else if (auto parseres = parseCondition())
		return parseres;
	else if (auto parseres = parseWhileLoop())
		return parseres;
	else if (auto parseres = parseCompoundStatement())
		return parseres;
	else
		return ParsingResult<IASTStmt>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTStmt> Parser::parseVarDeclStmt()
{
	//<var_decl> = <let_kw> <id> <type_spec> ['=' <expr>] ';'
	std::unique_ptr<IASTExpr> initExpr = 0;

	bool isVarConst = false;
	std::size_t varType = Types::InvalidIndex;
	std::string varName;

	if (matchKeyword(keyword::D_LET))
	{
		// ##ID##
		bool successfulMatchFlag = false;
		std::tie(
			successfulMatchFlag,
			varName
		) = matchID(); // get id

		if (!successfulMatchFlag)
		{
			errorExpected("Expected an identifier");
			if (resyncToDelimiter(sign::P_SEMICOLON))
				return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_BUT_RECOVERED);
			return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_AND_DIED);
		}
		// ##TYPESPEC##
		auto typespecResult = parseTypeSpec();
		// index 0 -> success flag
		// index 1 -> isConst flag
		// index 2 -> type index if success
		if (!std::get<0>(typespecResult))
		{
			errorExpected("Expected a ':'");
			if (resyncToDelimiter(sign::P_SEMICOLON))
				return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_BUT_RECOVERED);
			return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_AND_DIED);
		}
		else
		{
			// set variables
			isVarConst = std::get<1>(typespecResult);
			varType = std::get<2>(typespecResult);
		}

		// ##ASSIGNEMENT##
		// '=' <expr>
		if (matchSign(sign::S_EQUAL))
		{
			if (auto parseres = parseExpr())
				initExpr = std::move(parseres.node_);
			else
			{
				errorExpected("Expected an expression");
				if (resyncToDelimiter(sign::P_SEMICOLON))
					return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_BUT_RECOVERED);
				return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_AND_DIED);
			}
		}
		// ';'
		if (!matchSign(sign::P_SEMICOLON))
		{
			errorExpected("Expected semicolon after expression in variable declaration,");
			if (resyncToDelimiter(sign::P_SEMICOLON))
				return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_BUT_RECOVERED);
			return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_AND_DIED);
		}

		// If parsing was ok : 
		var::varattr v_attr(varName, varType, isVarConst);
		if (initExpr) // Has init expr?
			return ParsingResult<IASTStmt>(
				ParsingOutcome::SUCCESS,
				std::make_unique<ASTVarDeclStmt>(v_attr,std::move(initExpr))
			);
		else
			return ParsingResult<IASTStmt>(
				ParsingOutcome::SUCCESS,
				std::make_unique<ASTVarDeclStmt>(v_attr, nullptr)
			);
	}
	return ParsingResult<IASTStmt>(ParsingOutcome::NOTFOUND);
}

std::tuple<bool, bool, std::size_t> Parser::parseTypeSpec()
{
	bool isConst = false;
	std::size_t typ;
	if (matchSign(sign::P_COLON))
	{
		// Match const kw
		if (matchKeyword(keyword::T_CONST))
			isConst = true;
		// Now match the type keyword
		if ((typ = matchTypeKw()) != Types::InvalidIndex)
			return { true , isConst , typ };

		errorExpected("Expected a type name");
	}
	return { false, false, Types::InvalidIndex };
}

ParsingResult<IASTStmt> Parser::parseExprStmt()
{
	//<expr_stmt> = ';' |<expr> ';'
	if (matchSign(sign::P_SEMICOLON))
		return ParsingResult<IASTStmt>(ParsingOutcome::SUCCESS,
			std::make_unique<ASTNullStmt>()
		);
	else if (auto node = parseExpr())
	{
		// Found node
		if (matchSign(sign::P_SEMICOLON))
			return ParsingResult<IASTStmt>(ParsingOutcome::SUCCESS,std::move(node.node_));
		else
		{
			errorExpected("Expected a ';' in expression statement");
			if (resyncToDelimiter(sign::P_SEMICOLON))
				return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_BUT_RECOVERED);
			return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_AND_DIED);
		}
	}
	return ParsingResult<IASTStmt>(ParsingOutcome::NOTFOUND);
}