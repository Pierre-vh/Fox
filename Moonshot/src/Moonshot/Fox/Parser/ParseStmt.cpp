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
		return NULL_UNIPTR(IASTStmt);
}

std::unique_ptr<IASTStmt> Parser::parseVarDeclStmt()
{
	//<var_decl> = <let_kw> <id> <type_spec> ['=' <expr>] <eoi>
	std::unique_ptr<ASTExpr> initExpr = 0;

	bool isVarConst = false;
	std::size_t varType = invalid_index;
	std::string varName;

	if (matchKeyword(lex::D_LET))
	{
		// ##ID##
		bool successfulMatchFlag = false;
		std::tie(
			successfulMatchFlag,
			varName
		) = matchID(); // get id

		if (!successfulMatchFlag)
		{
			errorUnexpected();
			errorExpected("Expected an ID after \"let\" keyword");
		}
		// ##TYPESPEC##
		auto typespecResult = parseTypeSpec();
		// index 0 -> success flag
		// index 1 -> isConst flag
		// index 2 -> type index if success
		if (!std::get<0>(typespecResult))
		{
			errorUnexpected();
			errorExpected("Expected type specifier after ID");
		}
		else
		{
			// set variables
			isVarConst = std::get<1>(typespecResult);
			varType = std::get<2>(typespecResult);
		}
		
		// ##ASSIGNEMENT##
		// '=' <expr>
		if (matchSign(lex::S_EQUAL))
		{
			initExpr = parseExpr();
			if (!initExpr)
			{
				errorUnexpected();
				errorExpected("Expected expression after '=' sign");
			}
		}
		// ##EOI##
		if (!matchEOI())
		{
			errorUnexpected();
			errorExpected("Expected semicolon after expression in variable declaration");
		}
	}

	if (E_CHECKSTATE)
	{
		// If parsing was ok : 
		var::varattr v_attr(varName, varType, isVarConst);
		if (initExpr) // Has init expr?
			return std::make_unique<ASTVarDeclStmt>(v_attr,initExpr);
		return std::make_unique<ASTVarDeclStmt>(v_attr,NULL_UNIPTR(ASTExpr));
	}
	return NULL_UNIPTR(IASTStmt);
}

std::tuple<bool, bool, std::size_t> Parser::parseTypeSpec()
{
	bool isConst = false;
	std::size_t typ;
	if (matchSign(lex::P_COLON))
	{
		// Match const kw
		if (matchKeyword(lex::T_CONST))
			isConst = true;
		// Now match the type specifier
		if ((typ = matchTypeKw()) != invalid_index)
			return { true , isConst , typ };
		else
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
	return NULL_UNIPTR(IASTStmt);
}