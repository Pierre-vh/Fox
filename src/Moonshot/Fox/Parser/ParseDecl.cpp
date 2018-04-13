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
		<func_decl>		= "func" <id> '(' [<arg_decl_list>] ')'[':' <type>] <compound_statement>
		// Note about [':' <type>], if it isn't present, the function returns void
	*/

	// "func"
	if (matchKeyword(KeywordType::KW_FUNC))
	{
		auto rtr = std::make_unique<ASTFunctionDecl>();
		// <id>
		if (auto id = matchID())
			rtr->setFunctionIdentifier(id);
		else
		{
			errorExpected("Expected an identifier");
			return ParsingResult<ASTFunctionDecl*>(false);
		}

		// '('
		if (!matchSign(SignType::S_ROUND_OPEN))
		{
			errorExpected("Expected '('");
			// try to resync to a ')' without consuming it
			if (!resyncToSign(SignType::S_ROUND_CLOSE,/*consumeToken*/false))
				return ParsingResult<ASTFunctionDecl*>(false);
		}

		// [<arg_list_decl>]
		if (auto pArgDeclList = parseArgDeclList())
			rtr->setArgs(pArgDeclList.result);

		// ')'
		if (!matchSign(SignType::S_ROUND_CLOSE))
		{
			errorExpected("Expected a ')'");
			if (!resyncToSign(SignType::S_ROUND_CLOSE))
				return ParsingResult<ASTFunctionDecl*>(false);
		}
	
		// [':' <type>]
		if (matchSign(SignType::S_COLON))
		{
			if (auto rtrTy = parseTypeKw())
				rtr->setReturnType(rtrTy);
			else // no type found? we expected one after the colon!
			{
				errorExpected("Expected a type keyword");
				rtr->setReturnType(astCtxt_->getPrimitiveVoidType());
				// don't return just yet, wait to see if a { can be found so we can still return something.
				// return ParsingResult<ASTFunctionDecl*>(false);
			}
		}
		else // if no return type, the function returns void.
			rtr->setReturnType(astCtxt_->getPrimitiveVoidType());

		// <compound_statement>
		if (auto compstmt_res = parseCompoundStatement(/* mandatory = yes */ true, /* shouldn't attempt to recover (because recovery is handled by parseUnit)*/ false))
		{
			rtr->setBody(std::move(compstmt_res.result));
			// Success, nothing more to see here!
			return ParsingResult<ASTFunctionDecl*>(std::move(rtr));
		}
		else 
		{
			if (compstmt_res.wasSuccessful())
				errorExpected("Expected a {");
			return ParsingResult<ASTFunctionDecl*>(false);
		}
	}
	// not found
	return ParsingResult<ASTFunctionDecl*>();
}

ParsingResult<FunctionArg> Parser::parseArgDecl()
{
	// <arg_decl> = <id> <fq_type_spec>
	// <id>
	if (auto id = matchID())
	{
		// <fq_type_spec>
		if (auto typespec_res = parseFQTypeSpec())
			return ParsingResult<FunctionArg>(FunctionArg(id, typespec_res.result));
		else
		{
			if(typespec_res.wasSuccessful())		// not found, report an error
				errorExpected("Expected a ':'");
			// in both case (not found or error) return an error
			return ParsingResult<FunctionArg>(false);
		}
	}
	return ParsingResult<FunctionArg>();
}

ParsingResult<std::vector<FunctionArg>> Parser::parseArgDeclList()
{
	// <arg_decl_list> = [<arg_decl> {',' <arg_decl>}*]
	if (auto firstarg_res = parseArgDecl())
	{
		std::vector<FunctionArg> rtr;
		rtr.push_back(firstarg_res.result);
		// try to parse the rest
		while (true)
		{
			if (matchSign(SignType::S_COMMA))
			{
				if (auto argdecl_res = parseArgDecl())
					rtr.push_back(argdecl_res.result);
				else
				{
					if (argdecl_res.wasSuccessful()) // not found?
						errorExpected("Expected an argument declaration");
					return ParsingResult<std::vector<FunctionArg>>(false);
				}
			}
			else
				break;
		}
		return ParsingResult<std::vector<FunctionArg>>(rtr);
	}
	else
		return ParsingResult<std::vector<FunctionArg>>(firstarg_res.wasSuccessful());
}
ParsingResult<ASTVarDecl*> Parser::parseVarDeclStmt(const bool& recoverToSemiOnError)
{
	// <var_decl> = "let" <id> <fq_type_spec> ['=' <expr>] ';'
	// "let"
	if (matchKeyword(KeywordType::KW_LET))
	{
		auto rtr = std::make_unique<ASTVarDecl>();

		// <id>
		if (auto id = matchID())
			rtr->setVarIdentifier(id);
		else
		{
			errorExpected("Expected an identifier");
			// Recover to semicolon if allowed & return error
			if (recoverToSemiOnError)
				resyncToSign(SignType::S_SEMICOLON);
			return ParsingResult<ASTVarDecl*>(false);
		}

		// <fq_type_spec>
		if (auto typespecResult = parseFQTypeSpec())
		{
			QualType ty = std::move(typespecResult.result);
			if (ty.isAReference())
			{
				context_.reportWarning("Ignored reference qualifier '&' in variable declaration : Variables cannot be references.");
				ty.setIsReference(false);
			}
			rtr->setVarType(std::move(ty));
		}
		else
		{
			errorExpected("Expected a ':'");
			// Recover to semicolon if allowed & return error
			if (recoverToSemiOnError)
				resyncToSign(SignType::S_SEMICOLON);
			return ParsingResult<ASTVarDecl*>(false);
		}

		// ['=' <expr>]
		if (matchSign(SignType::S_EQUAL))
		{
			if (auto parseres = parseExpr())
				rtr->setInitExpr(std::move(parseres.result));
			else
			{
				if(parseres.wasSuccessful())
					errorExpected("Expected an expression");
				// Recover to semicolon if allowed & return error
				if (recoverToSemiOnError)
					resyncToSign(SignType::S_SEMICOLON);
				return ParsingResult<ASTVarDecl*>(false);
			}
		}

		// ';'
		if (!matchSign(SignType::S_SEMICOLON))
		{
			errorExpected("Expected semicolon after expression in variable declaration,");
			// Recover to semicolon if allowed & return error
			if (recoverToSemiOnError)
			{
				if(!resyncToSign(SignType::S_SEMICOLON))
					return ParsingResult<ASTVarDecl*>(false);
			}
			else
				return ParsingResult<ASTVarDecl*>(false);
		}
		// If we're here -> success
		return ParsingResult<ASTVarDecl*>(std::move(rtr));
	}
	// not found
	return ParsingResult<ASTVarDecl*>();
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
			return ParsingResult<QualType>(false);
		}

		// Success!
		return ParsingResult<QualType>(ty);
	}
	// not found!
	return ParsingResult<QualType>();
}

ParsingResult<ASTDecl*> Parser::parseDecl()
{
	// <declaration> = <var_decl> | <func_decl>

	// <var_decl>
	if (auto vdecl = parseVarDeclStmt(/* Don't recover on error */ false)) // we don't recover on error because recovery is handled by parseUnit.
		return ParsingResult<ASTDecl*>(std::move(vdecl.result));
	else if (!vdecl.wasSuccessful())
		return ParsingResult<ASTDecl*>(false);

	// <func_decl>
	if (auto fdecl = parseFunctionDeclaration())
		return ParsingResult<ASTDecl*>(std::move(fdecl.result));
	else if (!fdecl.wasSuccessful())
		return ParsingResult<ASTDecl*>(false);

	return ParsingResult<ASTDecl*>();
}