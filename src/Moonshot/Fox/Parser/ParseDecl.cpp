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
				if (!resyncToSign(SignType::S_ROUND_CLOSE))
					return ParsingResult<ASTFunctionDecl*>(ParsingOutcome::FAILED_AND_DIED);
			}
		}
		else
		{
			errorExpected("Expected '('");
			if (!resyncToSign(SignType::S_ROUND_CLOSE))
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
		if (auto cp_res = parseTopLevelCompoundStatement(true))
		{
			rtr->setBody(std::move(cp_res.result_));
			return ParsingResult<ASTFunctionDecl*>(ParsingOutcome::SUCCESS, std::move(rtr));
		}
		else
		{
			if (cp_res.isDataAvailable())
			{
				rtr->setBody(std::move(cp_res.result_));
				return ParsingResult<ASTFunctionDecl*>(cp_res.getFlag(), std::move(rtr));
			}
			else
			{
				// Create an empty compound statement to still return something
				rtr->setBody(std::make_unique<ASTCompoundStmt>());
				return ParsingResult<ASTFunctionDecl*>(cp_res.getFlag(),std::move(rtr));

			}
		}
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
ParsingResult<ASTVarDecl*> Parser::parseVarDeclStmt()
{
	auto node = parseTopLevelVarDeclStmt();
	// If failed w/o recovery, try to recover.
	if (node.getFlag() == ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY)
	{
		if (resyncToSign(SignType::S_SEMICOLON))
		{
			if(node.isDataAvailable())
				return ParsingResult<ASTVarDecl*>(ParsingOutcome::FAILED_BUT_RECOVERED, std::move(node.result_));
			return ParsingResult<ASTVarDecl*>(ParsingOutcome::FAILED_BUT_RECOVERED);
		}
		return ParsingResult<ASTVarDecl*>(ParsingOutcome::FAILED_AND_DIED);
	}
	return node;
}

ParsingResult<ASTVarDecl*> Parser::parseTopLevelVarDeclStmt()
{
	//<var_decl> = <let_kw> <id> <type_spec> ['=' <expr>] ';'
	std::unique_ptr<IASTExpr> initExpr = 0;
	ParsingOutcome flag = ParsingOutcome::SUCCESS;
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
			return ParsingResult<ASTVarDecl*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}
		// ##TYPESPEC##
		if (auto typespecResult = parseTypeSpec())
			varType = typespecResult.result_;
		else
		{
			errorExpected("Expected a ':'");
			return ParsingResult<ASTVarDecl*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
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
				return ParsingResult<ASTVarDecl*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
			}
		}
		// ';'
		if (!matchSign(SignType::S_SEMICOLON))
		{
			errorExpected("Expected semicolon after expression in variable declaration,");
			flag = ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY;
		}

		// If parsing was ok : 
		FoxVariableAttr v_attr(varName, varType);
		if (initExpr) // Has init expr?
			return ParsingResult<ASTVarDecl*>(
				flag,
					std::make_unique<ASTVarDecl>(v_attr, std::move(initExpr))
				);
		else
			return ParsingResult<ASTVarDecl*>(
				flag,
					std::make_unique<ASTVarDecl>(v_attr, nullptr)
				);
	}
	return ParsingResult<ASTVarDecl*>(ParsingOutcome::NOTFOUND);
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

ParsingResult<IASTDecl*> Parser::parseTopLevelDecl()
{
	// <declaration> = <var_decl> | <func_decl>
	// <var_decl>
	auto vdecl = parseTopLevelVarDeclStmt();
	if (vdecl.getFlag() != ParsingOutcome::NOTFOUND)
	{
		if (vdecl.isDataAvailable())
			return ParsingResult<IASTDecl*>(vdecl.getFlag(), std::move(vdecl.result_));
		return ParsingResult<IASTDecl*>(vdecl.getFlag());
	}
	// <func_decl>
	auto fdecl = parseFunctionDeclaration();
	if (fdecl.getFlag() != ParsingOutcome::NOTFOUND)
	{
		if(fdecl.isDataAvailable())
			return ParsingResult<IASTDecl*>(fdecl.getFlag(), std::move(fdecl.result_));
		return ParsingResult<IASTDecl*>(fdecl.getFlag());
	}

	return ParsingResult<IASTDecl*>(ParsingOutcome::NOTFOUND);
}
