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