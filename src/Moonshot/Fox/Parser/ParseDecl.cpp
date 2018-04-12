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

#include "Moonshot/Fox/Basic/Context.hpp"
#include "Moonshot/Fox/AST/ASTContext.hpp"
#include "Moonshot/Fox/AST/ASTDecl.hpp"
#include "Moonshot/Fox/AST/ASTStmt.hpp"

#include <cassert>

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
		{
			IdentifierInfo* id = astCtxt_->identifierTable().getUniqueIDInfoPtr(mID_res.result_);
			assert(id && "IdentifierTable returned a null IdentifierInfo?");
			rtr->setFunctionIdentifier(id);
		}
		else
		{
			errorExpected("Expected an identifier");
			return ParsingResult<ASTFunctionDecl*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
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
			if (Type* rtrTy = parseTypeKw())
				rtr->setReturnType(rtrTy);
			else
				errorExpected("Expected a type keyword");
		}
		else // if no return type, the function returns void.
			rtr->setReturnType(astCtxt_->getBuiltinVoidType());

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
				// If there was an error, create an empty compound statement to still return something
				rtr->setBody(std::make_unique<ASTCompoundStmt>());
				return ParsingResult<ASTFunctionDecl*>(cp_res.getFlag(),std::move(rtr));
			}
		}
	}
	return ParsingResult<ASTFunctionDecl*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<FunctionArg> Parser::parseArgDecl()
{
	// <arg_decl> = <id> <fq_type_spec>
	// <id>
	if (auto mID_res = matchID())
	{
		IdentifierInfo* id = astCtxt_->identifierTable().getUniqueIDInfoPtr(mID_res.result_);
		assert(id && "IdentifierTable returned a null IdentifierInfo?");
		// <fq_type_spec>
		if (auto typeSpec_res = parseFQTypeSpec())
			return ParsingResult<FunctionArg>(ParsingOutcome::SUCCESS, FunctionArg(id, typeSpec_res.result_));

		// failure cases
		else if (typeSpec_res.getFlag() == ParsingOutcome::NOTFOUND)
			errorExpected("Expected a ':'");

		return ParsingResult<FunctionArg>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
	}
	return ParsingResult<FunctionArg>(ParsingOutcome::NOTFOUND);
}

ParsingResult<std::vector<FunctionArg>> Parser::parseArgDeclList()
{
	if (auto firstArg_res = parseArgDecl())
	{
		std::vector<FunctionArg> rtr;
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
					return ParsingResult<std::vector<FunctionArg>>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
				}
			}
			else
				break;
		}
		return ParsingResult<std::vector<FunctionArg>>(ParsingOutcome::SUCCESS, rtr);
	}
	else
		return ParsingResult<std::vector<FunctionArg>>(firstArg_res.getFlag());
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
	// <var_decl> = "let" <id> <fq_type_spec> ['=' <expr>] ';'
	std::unique_ptr<ASTExpr> initExpr = 0;
	ParsingOutcome flag = ParsingOutcome::SUCCESS;

	QualType varType;
	IdentifierInfo *varId = nullptr;
	// "let"
	if (matchKeyword(KeywordType::KW_LET))
	{
		// <id>
		if (auto match = matchID())
			varId = astCtxt_->identifierTable().getUniqueIDInfoPtr(match.result_);
		else
		{
			errorExpected("Expected an identifier");
			return ParsingResult<ASTVarDecl*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}

		// <fq_type_spec>
		if (auto typespecResult = parseFQTypeSpec())
		{
			auto qualTy = typespecResult.result_;
			if (qualTy.isAReference())
			{
				context_.reportWarning("Ignored reference qualifier '&' in variable declaration : Variables cannot be references.");
				qualTy.setIsReference(false);
			}
			varType = qualTy;
		}
		else
		{
			errorExpected("Expected a ':'");
			return ParsingResult<ASTVarDecl*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}

		// ['=' <expr>]
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
		if (initExpr) // Has init expr?
			return ParsingResult<ASTVarDecl*>(
				flag,
					std::make_unique<ASTVarDecl>(varId,varType, std::move(initExpr))
				);
		else
			return ParsingResult<ASTVarDecl*>(
				flag,
					std::make_unique<ASTVarDecl>(varId,varType, nullptr)
				);
	}
	return ParsingResult<ASTVarDecl*>(ParsingOutcome::NOTFOUND);
}

ParsingResult<QualType> Parser::parseFQTypeSpec()
{
	// 	<fq_type_spec>	= ':' ["const"] ['&'] <type>
	if (matchSign(SignType::S_COLON))
	{
		QualType ty;
		// ["const"]
		if (matchKeyword(KeywordType::KW_CONST))
			ty.setConstAttribute(true);

		// ['&']
		if (matchSign(SignType::S_AMPERSAND))
			ty.setIsReference(true);

		// <type>
		if (auto type = parseTypeKw())
			ty.setType(type);
		else
		{
			errorExpected("Expected a type name");
			return ParsingResult<QualType>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY);
		}

		return ParsingResult<QualType>(ParsingOutcome::SUCCESS, ty);
	}
	return ParsingResult<QualType>(ParsingOutcome::NOTFOUND);
}

ParsingResult<ASTDecl*> Parser::parseTopLevelDecl()
{
	// <declaration> = <var_decl> | <func_decl>
	// <var_decl>
	auto vdecl = parseTopLevelVarDeclStmt();
	if (vdecl.getFlag() != ParsingOutcome::NOTFOUND)
	{
		if (vdecl.isDataAvailable())
			return ParsingResult<ASTDecl*>(vdecl.getFlag(), std::move(vdecl.result_));
		return ParsingResult<ASTDecl*>(vdecl.getFlag());
	}
	// <func_decl>
	auto fdecl = parseFunctionDeclaration();
	if (fdecl.getFlag() != ParsingOutcome::NOTFOUND)
	{
		if(fdecl.isDataAvailable())
			return ParsingResult<ASTDecl*>(fdecl.getFlag(), std::move(fdecl.result_));
		return ParsingResult<ASTDecl*>(fdecl.getFlag());
	}

	return ParsingResult<ASTDecl*>(ParsingOutcome::NOTFOUND);
}
