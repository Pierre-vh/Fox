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
using namespace fv_util;

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

std::unique_ptr<IASTStmt> Parser::parseCompoundStatement()
{
	auto rtr = std::make_unique<ASTCompStmt>(); // return value
	if (matchSign(sign::B_CURLY_OPEN))
	{
		// Parse all statements
		while (auto node = parseStmt())
		{
			if (rtr->statements_.size())
			{
				// Don't push another null statement if the last statement is already a null one.
				if (dynamic_cast<ASTNullStmt*>(rtr->statements_.back().get()) &&
					dynamic_cast<ASTNullStmt*>(node.get()))
					continue;
			}
			rtr->statements_.push_back(std::move(node));
		}
		// Match the closing curly bracket
		if (!matchSign(sign::B_CURLY_CLOSE))
		{
			errorExpected("Expected a closing curly bracket '}' at the end of the compound statement,");
			if(!resyncToDelimiter(sign::B_CURLY_CLOSE))
				return nullptr;
		}
		return rtr;
	}
	return nullptr;
}

std::unique_ptr<IASTStmt> Parser::parseWhileLoop()
{
	// Rule : <while_loop> 	= <wh_kw>  '(' <expr> ')'	<compound_statement> 
	if (matchKeyword(keyword::D_WHILE))
	{
		std::unique_ptr<ASTWhileLoop> rtr = std::make_unique<ASTWhileLoop>();
		// (
		if (!matchSign(sign::B_ROUND_OPEN))
		{
			errorExpected("Expected a '('");
			return nullptr;
		}
		// expr
		if (auto node = parseExpr())
			rtr->expr_ = std::move(node);
		else
		{
			errorExpected("Expected an expression after '(' in while loop declaration");
			return nullptr;
		}
		// )
		if (!matchSign(sign::B_ROUND_CLOSE))
		{
			errorExpected("Expected a ')' after expression in while statement");
			if(!resyncToDelimiter(sign::B_ROUND_CLOSE))
				return nullptr;
		}
		// <compound_statement>
		if (auto node = parseStmt())
			rtr->body_ = std::move(node);
		else
		{
			errorExpected("Expected a Statement after while loop declaration");
			return nullptr;
		}
		// Return
		return rtr;
	}
	return nullptr;
}

std::unique_ptr<IASTStmt> Parser::parseCondition()
{
	auto rtr = std::make_unique<ASTCondition>();
	auto result = parseCond_if();

	if (result.isComplete())
	{
		// <cond_if>
		rtr->conditional_stmts_.push_back({
			result.resetAndReturnTmp()
		});
		// <cond_elif>
		auto elif_res = parseCond_elseIf();
		while (elif_res.isComplete()) // consume all elifs
		{
			rtr->conditional_stmts_.push_back({
				elif_res.resetAndReturnTmp()
			});
			// try to find another elif
			elif_res = parseCond_elseIf();
		}
		// <cond_else>
		if (auto node = parseCond_else()) // it's a else
			rtr->else_stmt_ = std::move(node);

		return rtr;
	}
	// change this bit in the new system to
	else if (parseCond_elseIf().isComplete())		// if parsing of else if successful	
		genericError("Else if without matching if.");
	else if (auto node = parseCond_else())			// if parsing of else successful
		genericError("Else without matching if.");
	return nullptr;
}

ConditionalStatement Parser::parseCond_if()
{
	// "if"
	if (matchKeyword(keyword::D_IF))
	{
		ConditionalStatement rtr;
		// '('
		if (!matchSign(sign::B_ROUND_OPEN))
		{
			errorExpected("Expected a '('");
			return ConditionalStatement();
		}
		// <expr>
		if (auto node = parseExpr())
			rtr.expr_ = std::move(node);
		else
		{
			errorExpected("Expected an expression after '(' in if condition,");
			return ConditionalStatement();
		}
		// ')'
		if (!matchSign(sign::B_ROUND_CLOSE))
		{
			errorExpected("Expected a ')' after expression in if condition,");
			if(!resyncToDelimiter(sign::B_ROUND_CLOSE))
				return ConditionalStatement();
		}
		// <statement>
		if (auto node = parseStmt())
			rtr.stmt_ = std::move(node);
		else
		{
			errorExpected("Expected a statement after if condition,");
			return ConditionalStatement();
		}
		// Finished, return.
		return rtr;
	}
	return ConditionalStatement();
}

ConditionalStatement Parser::parseCond_elseIf()
{
	ConditionalStatement rtr;
	auto bckp = createParserStateBackup();
	if (matchKeyword(keyword::D_ELSE))
	{
		if (matchKeyword(keyword::D_IF))
		{
			// '('
			if (!matchSign(sign::B_ROUND_OPEN))
			{
				errorExpected("Expected a '('");
				// Try to parse a statement to give further error messages.
				parseStmt();
			}
			// <expr>
			if (auto node = parseExpr())
				rtr.expr_ = std::move(node);
			else
			{
				errorExpected("Expected an expression after '(' in else if condition,");
				return ConditionalStatement();
			}
			// ')'
			if (!matchSign(sign::B_ROUND_CLOSE))
			{
				errorExpected("Expected a ')' after expression in else if condition,");
				if(!resyncToDelimiter(sign::B_ROUND_CLOSE))
					return ConditionalStatement();
			}
			// <statement>
			if (auto node = parseStmt())
			{
				rtr.stmt_ = std::move(node);
				return rtr;
			}
			else 
			{
				errorExpected("Expected a statement after else if condition,");
				return ConditionalStatement();
			}
		}
		// restore a backup, because if the leave the "else" consumed, it won't be picked up
		// by the "parseCond_else" function later in parseCond.
		restoreParserStateFromBackup(bckp);
		return ConditionalStatement();
	}
	return ConditionalStatement();
}

std::unique_ptr<IASTStmt> Parser::parseCond_else()
{
	if (matchKeyword(keyword::D_ELSE))
	{
		if (auto node = parseStmt())
			return node;
		else
		{
			errorExpected("Expected a statement");
			return nullptr;
		}
	}
	return nullptr;
}


std::unique_ptr<IASTStmt> Parser::parseStmt()
{
	// <stmt>	= <var_decl> | <expr_stmt> | <condition> | <while_loop> | <compound_statement> | (<rtr_stmt> -> to be implemented)
	std::unique_ptr<IASTStmt> node;
	if (node = parseExprStmt())
		return node;
	else if (node = parseVarDeclStmt())
		return node;
	else if (node = parseCondition())
		return node;
	else if (node = parseWhileLoop())
		return node;
	else if (node = parseCompoundStatement())
		return node;
	else
		return nullptr;
}

std::unique_ptr<IASTStmt> Parser::parseVarDeclStmt()
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
			resyncToDelimiter(sign::P_SEMICOLON);
			return nullptr;
		}
		// ##TYPESPEC##
		auto typespecResult = parseTypeSpec();
		// index 0 -> success flag
		// index 1 -> isConst flag
		// index 2 -> type index if success
		if (!std::get<0>(typespecResult))
		{
			errorExpected("Expected a type specifier");
			resyncToDelimiter(sign::P_SEMICOLON); // Resync to semicolon before returning
			return nullptr;
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
			initExpr = parseExpr();
			if (!initExpr)
			{
				errorExpected("Expected an expression");
				resyncToDelimiter(sign::P_SEMICOLON); // Resync to semicolon before returning
				return nullptr;
			}
		}
		// ';'
		if (!matchSign(sign::P_SEMICOLON))
		{
			errorExpected("Expected semicolon after expression in variable declaration,");
			resyncToDelimiter(sign::P_SEMICOLON); // Resync to semicolon before returning
			return nullptr;
		}

		// If parsing was ok : 
		var::varattr v_attr(varName, varType, isVarConst);
		if (initExpr) // Has init expr?
			return std::make_unique<ASTVarDeclStmt>(v_attr, initExpr);
		else
			return std::make_unique<ASTVarDeclStmt>(v_attr, std::unique_ptr<IASTExpr>(nullptr));
	}
	return nullptr;
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

std::unique_ptr<IASTStmt> Parser::parseExprStmt()
{
	//<expr_stmt> = ';' |<expr> ';'
	if (matchSign(sign::P_SEMICOLON))
		return std::make_unique<ASTNullStmt>();
	else if (auto node = parseExpr())
	{
		// Found node
		if (matchSign(sign::P_SEMICOLON))
			return node;
		else
		{
			errorExpected("Expected a ';' in expression statement");
			resyncToDelimiter(sign::P_SEMICOLON); // Discard all tokens until it's found, and continue parsing.
		}
	}

	return nullptr;
}