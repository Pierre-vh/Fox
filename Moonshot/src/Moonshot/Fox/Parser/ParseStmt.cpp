////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ParseStmt.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
//	This file implements statements rules. parseStmt, parseVarDeclstmt,etc.									
////------------------------------------------------------////

#include "Parser.h"

using namespace Moonshot;
using namespace fv_util;

std::unique_ptr<ASTCompStmt> Parser::parseCompoundStatement()
{
	/*
		<compound_statement> = '{' {<stmt>} '}'		
		| = <stmt>	
	*/
	// return value
	auto rtr = std::make_unique<ASTCompStmt>();
	if (matchSign(signType::B_CURLY_OPEN))
	{
		// Parse all statements
		while (auto node = parseStmt())
			rtr->statements_.push_back(std::move(node));
		// Match the closing curly bracket
		if (!matchSign(signType::B_CURLY_CLOSE))
		{
			errorUnexpected();
			errorExpected("Expected a closing curly bracket '}' at the end of the compound statement.");
			return nullptr;
		}
		// Return
		return rtr;
	}
	else if (auto node = parseStmt())
	{
		// parse the statement + return
		rtr->statements_.push_back(std::move(node));
		return rtr;
	}
	return nullptr;
}

std::unique_ptr<ASTCondition> Parser::parseCondition()
{
	auto rtr = std::make_unique<ASTCondition>();
	auto result = parseCond_if();
	if (result.first && result.second)
	{
		// we need to use moves to push the new values in
		rtr->conditional_blocks_.push_back({
			std::move(result.first),
			std::move(result.second)
		});
		auto elif_res = parseCond_else_if();
		while (elif_res.first && elif_res.second) // consume all elifs
		{
			// we need to use moves to push the new values in
			rtr->conditional_blocks_.push_back({ 
				std::move(elif_res.first),
				std::move(elif_res.second) 
			});
			// try to find another elif
			elif_res = parseCond_else_if();
		}
		if ((!elif_res.first) && elif_res.second) // it's a else
			rtr->else_block_ = std::move(elif_res.second);

		return rtr;
	}
	else if (result.first || result.second)
		throw Exceptions::parser_critical_error("parseCond_if() returned a invalid CondBlock!");
	else
		return nullptr;
}

ASTCondition::CondBlock Parser::parseCond_if()
{
	ASTCondition::CondBlock rtr;
	// "if"
	if (matchKeyword(keywordType::D_IF))
	{
		// '('
		if (!matchSign(signType::B_ROUND_OPEN))
		{
			errorUnexpected();
			errorExpected("Expected a round bracket '(' after \"if\" keyword.");
			return { nullptr, nullptr };
		}
		// <expr>
		if (auto node = parseExpr())
			rtr.first = std::move(node);
		else
		{
			errorUnexpected();
			errorExpected("Expected an expression after '(' in condition.");
			return { nullptr, nullptr };
		}
		// ')'
		if (!matchSign(signType::B_ROUND_CLOSE))
		{
			errorUnexpected();
			errorExpected("Expected a round bracket ')' after expression in condition.");
			return { nullptr, nullptr };
		}
		// <compound_statement>
		if (auto node = parseCompoundStatement())
			rtr.second = std::move(node);
		else
		{
			errorExpected("Expected compound statement in condition");
			return { nullptr, nullptr };
		}
		// Finished, return.
		return rtr;
	}
	return { nullptr, nullptr };
}

ASTCondition::CondBlock Parser::parseCond_else_if()
{
	ASTCondition::CondBlock rtr;
	if (matchKeyword(keywordType::D_ELSE))
	{
		// else if
		auto res = parseCond_if();
		if (res.first && res.second) // parsed OK
		{
			rtr.first = std::move(res.first);
			rtr.second = std::move(res.second);
			return rtr;
		}
		else if (res.first || res.second)
			throw Exceptions::parser_critical_error("parseCond_if() returned a invalid CondBlock!");
		// Else
		else if (auto node = parseCompoundStatement())
		{
			rtr.second = std::move(node);
			return rtr;
		}
		// error case
		else
		{
			errorUnexpected();
			errorExpected("Expected a if condition OR a compound statement after \"else\" keyword.");
			return { nullptr, nullptr };
		}
	}
	return { nullptr, nullptr };
}


std::unique_ptr<IASTStmt> Parser::parseStmt()
{
	// <stmt> = <var_decl> | <expr_stmt> | <ctrl_flow>
	std::unique_ptr<IASTStmt> node;
	if (node = parseExprStmt())
		return node;
	else if (node = parseVarDeclStmt())
		return node;
	else
		return nullptr;
}

std::unique_ptr<IASTStmt> Parser::parseVarDeclStmt()
{
	//<var_decl> = <let_kw> <id> <type_spec> ['=' <expr>] <eoi>
	std::unique_ptr<ASTExpr> initExpr = 0;

	bool isVarConst = false;
	std::size_t varType = invalid_index;
	std::string varName;

	if (matchKeyword(keywordType::D_LET))
	{
		// ##ID##
		bool successfulMatchFlag = false;
		std::tie(
			successfulMatchFlag,
			varName
		) = matchID(); // get id

		if (!successfulMatchFlag)
		{
			if (context_.isSafe())
			{
				errorUnexpected();
				errorExpected("Expected an ID after \"let\" keyword");
			}
		}
		// ##TYPESPEC##
		auto typespecResult = parseTypeSpec();
		// index 0 -> success flag
		// index 1 -> isConst flag
		// index 2 -> type index if success
		if (!std::get<0>(typespecResult))
		{
			if (context_.isSafe())
			{
				errorUnexpected();
				errorExpected("Expected type specifier after ID");
			}
		}
		else
		{
			// set variables
			isVarConst = std::get<1>(typespecResult);
			varType = std::get<2>(typespecResult);
		}

		// ##ASSIGNEMENT##
		// '=' <expr>
		if (matchSign(signType::S_EQUAL))
		{
			initExpr = parseExpr();
			if (!initExpr)
			{
				if (context_.isSafe())
				{
					errorUnexpected();
					errorExpected("Expected expression after '=' sign");
				}
			}
		}
		// ##EOI##
		if (!matchEOI())
		{
			if (context_.isSafe())
			{
				errorUnexpected();
				errorExpected("Expected semicolon after expression in variable declaration");
			}
		}
		if (context_.isSafe())
		{
			// If parsing was ok : 
			var::varattr v_attr(varName, varType, isVarConst);
			if (initExpr) // Has init expr?
				return std::make_unique<ASTVarDeclStmt>(v_attr, initExpr);
			else
				return std::make_unique<ASTVarDeclStmt>(v_attr, std::unique_ptr<ASTExpr>(nullptr));
		}
	}
	return nullptr;
}

std::tuple<bool, bool, std::size_t> Parser::parseTypeSpec()
{
	bool isConst = false;
	std::size_t typ;
	if (matchSign(signType::P_COLON))
	{
		// Match const kw
		if (matchKeyword(keywordType::T_CONST))
			isConst = true;
		// Now match the type specifier
		if ((typ = matchTypeKw()) != invalid_index)
			return { true , isConst , typ };
		else if (context_.isSafe())
			errorExpected("Expected type keyword in type specifier.");
	}
	return { false, false, invalid_index };
}

std::unique_ptr<IASTStmt> Parser::parseExprStmt()
{
	//<expr_stmt> = <expr> <eoi>
	auto node = parseExpr();
	if (node)
	{
		// Found node
		if (matchEOI())
			return node;
		else
			errorExpected("Expected a semicolon after expression in expressionStatement.");
	}
	else if (matchEOI())
	{
		errorExpected("Expected an expression before semicolon.");
	}
	return nullptr;
}