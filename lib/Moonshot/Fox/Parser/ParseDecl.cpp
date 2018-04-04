////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ParseDecl.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
// This file implements decl, declstmt rules (methods)
// and related helper functions
////------------------------------------------------------////

#include "Parser.hpp"
// Needed nodes
#include "Moonshot/Fox/AST/ASTDecl.hpp"
#include "Moonshot/Fox/AST/ASTStmt.hpp"

using namespace Moonshot;

ParsingResult<ASTFunctionDecl*> Parser::parseFunctionDeclaration()
{
	/*
		<func_decl> = "func" <id> '(' [<arg_list_decl>] ')'[':' <type>] <compound_statement>	// Note about type_spec : if it is not present, the function returns void.
	*/
	// "func"
	if (matchKeyword(KeywordType::KW_FUNC))
	{
		auto rtr = std::make_unique<ASTFunctionDecl>();
		// <id>
		if (auto mID_res = matchID())
			rtr->setName(mID_res.result_);
		else
		{
			rtr->setName("<noname>");
			errorExpected("Expected an identifier");
		}

		// '('
		if (matchSign(SignType::S_ROUND_OPEN))
		{
			// [<arg_list_decl>]
			auto pArgDeclList = parseArgDeclList();
			if (pArgDeclList)
				rtr->setArgs(pArgDeclList.result_);
			// ')'
			if (!matchSign(SignType::S_ROUND_CLOSE))
			{
				if (pArgDeclList.getFlag() != ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY)
					errorExpected("Expected a ')'");
				if (!resyncToDelimiter(SignType::S_ROUND_CLOSE))
					return ParsingResult<ASTFunctionDecl*>(ParsingOutcome::FAILED_AND_DIED);
			}
		}
		else
		{
			errorExpected("Expected '('");
			if (!resyncToDelimiter(SignType::S_ROUND_CLOSE))
				return ParsingResult<ASTFunctionDecl*>(ParsingOutcome::FAILED_AND_DIED);
		}
		// [':' <type>]
		if (matchSign(SignType::S_COLON))
		{
			if (auto tyMatchRes = matchTypeKw())
				rtr->setReturnType(tyMatchRes.result_);
			else
				errorExpected("Expected a type keyword");
		}
		else
			rtr->setReturnType(TypeIndex::Void_Type);

		// <compound_statement>
		if (auto cp_res = parseCompoundStatement(true))
		{
			rtr->setBody(std::move(cp_res.result_));
			return ParsingResult<ASTFunctionDecl*>(ParsingOutcome::SUCCESS, std::move(rtr));
		}
		else
			return ParsingResult<ASTFunctionDecl*>(cp_res.getFlag());
	}
	return ParsingResult<ASTFunctionDecl*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<FoxFunctionArg> Parser::parseArgDecl()
{
	// <id>
	if (auto mID_res = matchID())
	{
		FoxFunctionArg rtr;
		rtr.setName(mID_res.result_);
		// ':'
		if (!matchSign(SignType::S_COLON))
		{
			errorExpected("Expected ':'");
			return ParsingResult<FoxFunctionArg>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
		// ["const"]
		if (matchKeyword(KeywordType::KW_CONST))
			rtr.setConst(true);
		// ['&']
		if (matchSign(SignType::S_AMPERSAND))
			rtr.setIsRef(true);
		else
			rtr.setIsRef(false);

		if (auto mty_res = matchTypeKw())
		{
			rtr.setType(mty_res.result_);
			return ParsingResult<FoxFunctionArg>(ParsingOutcome::SUCCESS, rtr);
		}
		else
		{
			errorExpected("Expected type name");
			return ParsingResult<FoxFunctionArg>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
	}
	return ParsingResult<FoxFunctionArg>(ParsingOutcome::NOTFOUND);
}

ParsingResult<std::vector<FoxFunctionArg>> Parser::parseArgDeclList()
{
	if (auto firstArg_res = parseArgDecl())
	{
		std::vector<FoxFunctionArg> rtr;
		rtr.push_back(firstArg_res.result_);
		while (true)
		{
			if (matchSign(SignType::S_COMMA))
			{
				if (auto pArgDecl_res = parseArgDecl())
					rtr.push_back(pArgDecl_res.result_);
				else
				{
					if (pArgDecl_res.getFlag() == ParsingOutcome::NOTFOUND)
						errorExpected("Expected an argument declaration");
					return ParsingResult<std::vector<FoxFunctionArg>>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
				}
			}
			else
				break;
		}
		return ParsingResult<std::vector<FoxFunctionArg>>(ParsingOutcome::SUCCESS, rtr);
	}
	else
		return ParsingResult<std::vector<FoxFunctionArg>>(firstArg_res.getFlag());
}

ParsingResult<IASTStmt*> Parser::parseVarDeclStmt()
{
	//<var_decl> = <let_kw> <id> <type_spec> ['=' <expr>] ';'
	std::unique_ptr<IASTExpr> initExpr = 0;

	bool isVarConst = false;
	FoxType varType = TypeIndex::InvalidIndex;
	std::string varName;

	if (matchKeyword(KeywordType::KW_LET))
	{
		// ##ID##
		if (auto match = matchID())
		{
			varName = match.result_;
		}
		else
		{
			errorExpected("Expected an identifier");
			if (resyncToDelimiter(SignType::S_SEMICOLON))
				return ParsingResult<IASTStmt*>(ParsingOutcome::FAILED_BUT_RECOVERED);
			return ParsingResult<IASTStmt*>(ParsingOutcome::FAILED_AND_DIED);
		}
		// ##TYPESPEC##
		if (auto typespecResult = parseTypeSpec())
			varType = typespecResult.result_;
		else
		{
			errorExpected("Expected a ':'");
			if (resyncToDelimiter(SignType::S_SEMICOLON))
				return ParsingResult<IASTStmt*>(ParsingOutcome::FAILED_BUT_RECOVERED);
			return ParsingResult<IASTStmt*>(ParsingOutcome::FAILED_AND_DIED);
		}

		// ##ASSIGNEMENT##
		// '=' <expr>
		if (matchSign(SignType::S_EQUAL))
		{
			if (auto parseres = parseExpr())
				initExpr = std::move(parseres.result_);
			else
			{
				errorExpected("Expected an expression");
				if (resyncToDelimiter(SignType::S_SEMICOLON))
					return ParsingResult<IASTStmt*>(ParsingOutcome::FAILED_BUT_RECOVERED);
				return ParsingResult<IASTStmt*>(ParsingOutcome::FAILED_AND_DIED);
			}
		}
		// ';'
		if (!matchSign(SignType::S_SEMICOLON))
		{
			errorExpected("Expected semicolon after expression in variable declaration,");
			if (resyncToDelimiter(SignType::S_SEMICOLON))
				return ParsingResult<IASTStmt*>(ParsingOutcome::FAILED_BUT_RECOVERED);
			return ParsingResult<IASTStmt*>(ParsingOutcome::FAILED_AND_DIED);
		}

		// If parsing was ok : 
		FoxVariableAttr v_attr(varName, varType);
		if (initExpr) // Has init expr?
			return ParsingResult<IASTStmt*>(
				ParsingOutcome::SUCCESS,
				std::make_unique<ASTVarDecl>(v_attr, std::move(initExpr))
				);
		else
			return ParsingResult<IASTStmt*>(
				ParsingOutcome::SUCCESS,
				std::make_unique<ASTVarDecl>(v_attr, nullptr)
				);
	}
	return ParsingResult<IASTStmt*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<FoxType> Parser::parseTypeSpec()
{
	bool isConst = false;
	if (matchSign(SignType::S_COLON))
	{
		// Match const kw
		if (matchKeyword(KeywordType::KW_CONST))
			isConst = true;
		// Now match the type keyword
		if (auto mTy_res = matchTypeKw())
			return ParsingResult<FoxType>(ParsingOutcome::SUCCESS, FoxType(mTy_res.result_, isConst));

		errorExpected("Expected a type name");
	}
	return ParsingResult<FoxType>(ParsingOutcome::NOTFOUND);
}