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
using namespace TypeUtils;

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
		// (
		if (!matchSign(sign::B_ROUND_OPEN))
		{
			errorExpected("Expected a '('");
			return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
		// expr
		if (auto parseres = parseExpr())
			rtr->expr_ = std::move(parseres.node_);
		else
		{
			errorExpected("Expected an expression after '(' in while loop declaration");
			return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
		// )
		if (!matchSign(sign::B_ROUND_CLOSE))
		{
			errorExpected("Expected a ')' after expression in while statement");
			if(!resyncToDelimiter(sign::B_ROUND_CLOSE))
				return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_BUT_RECOVERED);
			return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_AND_DIED);
		}
		// <stmt>
		if (auto parseres = parseStmt())
			rtr->body_ = std::move(parseres.node_);
		else
		{
			errorExpected("Expected a Statement after while loop declaration");
			return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
		// Return
		return ParsingResult<IASTStmt>(ParsingOutcome::SUCCESS, std::move(rtr));
	}
	return ParsingResult<IASTStmt>(ParsingOutcome::NOTFOUND);
}

ParsingResult<IASTStmt> Parser::parseCondition()
{
	auto rtr = std::make_unique<ASTCondition>();
	if(auto result = parseCond_if())
	{
		if (result.node_->isComplete())
		{
			// <cond_if>
			rtr->conditional_stmts_.push_back({
				result.node_->resetAndReturnTmp()
				});
			// <cond_elif>
			auto elif_res = parseCond_elseIf();
			while (elif_res && elif_res.node_->isComplete()) // consume all elifs
			{
				rtr->conditional_stmts_.push_back({
					elif_res.node_->resetAndReturnTmp()
					});
				// try to find another elif
				elif_res = parseCond_elseIf();
			}
			// <cond_else>
			if (auto parseres = parseCond_else()) // it's a else
				rtr->else_stmt_ = std::move(parseres.node_);

			return ParsingResult<IASTStmt>(ParsingOutcome::SUCCESS, std::move(rtr));
		}
		return ParsingResult<IASTStmt>(result.getFlag());
	}
	// change this bit in the new system to
	else if (parseCond_elseIf())		// if parsing of else if successful	
	{
		genericError("Else if without matching if.");
		return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_BUT_RECOVERED,std::make_unique<ASTNullStmt>());
	}
	else if (parseCond_else())			// if parsing of else successful
	{
		genericError("Else without matching if.");
		return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_BUT_RECOVERED, std::make_unique<ASTNullStmt>());
	}
	return ParsingResult<IASTStmt>(ParsingOutcome::NOTFOUND);
}

ParsingResult<ConditionalStatement> Parser::parseCond_if()
{
	// "if"
	if (matchKeyword(keyword::D_IF))
	{
		auto rtr = std::make_unique<ConditionalStatement>();
		// '('
		if (!matchSign(sign::B_ROUND_OPEN))
		{
			errorExpected("Expected a '('");
			return ParsingResult<ConditionalStatement>(
				ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY
			);
		}
		// <expr>
		if (auto parseres = parseExpr())
			rtr->expr_ = std::move(parseres.node_);
		else
		{
			errorExpected("Expected an expression after '(' in if condition,");
			return ParsingResult<ConditionalStatement>(
				ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY
			);
		}
		// ')'
		if (!matchSign(sign::B_ROUND_CLOSE))
		{
			errorExpected("Expected a ')' after expression in if condition,");
			if(resyncToDelimiter(sign::B_ROUND_CLOSE))
				return ParsingResult<ConditionalStatement>(ParsingOutcome::FAILED_BUT_RECOVERED);
			return ParsingResult<ConditionalStatement>(ParsingOutcome::FAILED_AND_DIED);
		}
		// <statement>
		if (auto parseres = parseStmt())
			rtr->stmt_ = std::move(parseres.node_);
		else
		{
			errorExpected("Expected a statement after if condition,");
			return ParsingResult<ConditionalStatement>(
				ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY
			);
		}
		// Finished, return.
		return ParsingResult<ConditionalStatement>(
			ParsingOutcome::SUCCESS,
			std::move(rtr)
		);
	}
	return ParsingResult<ConditionalStatement>(ParsingOutcome::NOTFOUND);
}

ParsingResult<ConditionalStatement> Parser::parseCond_elseIf()
{
	if (matchKeyword(keyword::D_ELSE))
	{
		auto bckp = createParserStateBackup();

		if (matchKeyword(keyword::D_IF))
		{
			auto rtr = std::make_unique<ConditionalStatement>();
			// '('
			if (!matchSign(sign::B_ROUND_OPEN))
				errorExpected("Expected a '('");

			// <expr>
			if (auto parseres = parseExpr())
				rtr->expr_ = std::move(parseres.node_);
			else
				errorExpected("Expected an expression after '(' in else if condition,");

			// ')'
			if (!matchSign(sign::B_ROUND_CLOSE))
			{
				errorExpected("Expected a ')' after expression in else if condition,");
				if (!resyncToDelimiter(sign::B_ROUND_CLOSE))
					return ParsingResult<ConditionalStatement>(ParsingOutcome::FAILED_AND_DIED);
			}
			// <stmt>
			if (auto parseres = parseStmt())
			{
				rtr->stmt_ = std::move(parseres.node_);
				return ParsingResult<ConditionalStatement>(
					ParsingOutcome::SUCCESS,
					std::move(rtr)
				);
			}
			else 
			{
				errorExpected("Expected a statement after else if condition,");
				return ParsingResult<ConditionalStatement>(
					ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY
				);
			}
		}
		// restore a backup, because if the leave the "else" consumed, it won't be picked up
		// by the "parseCond_else" function later in parseCond.
		restoreParserStateFromBackup(bckp);
		return ParsingResult<ConditionalStatement>(
			ParsingOutcome::NOTFOUND
		);
	}
	return ParsingResult<ConditionalStatement>(
		ParsingOutcome::NOTFOUND
	);
}

ParsingResult<IASTStmt> Parser::parseCond_else()
{
	if (matchKeyword(keyword::D_ELSE))
	{
		if (auto parseres = parseStmt())
			return ParsingResult<IASTStmt>(ParsingOutcome::SUCCESS,std::move(parseres.node_));
		else
		{
			errorExpected("Expected a statement");
			return ParsingResult<IASTStmt>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
	}
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
	std::size_t varType = indexes::invalid_index;
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
			errorExpected("Expected an ID");
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
			errorExpected("Expected a type specifier");
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
		if ((typ = matchTypeKw()) != indexes::invalid_index)
			return { true , isConst , typ };

		errorExpected("Expected a valid type keyword in type specifier");
	}
	return { false, false, indexes::invalid_index };
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