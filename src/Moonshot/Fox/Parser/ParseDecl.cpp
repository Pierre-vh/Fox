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
		<func_decl>		= "func" <id> '(' [<arg_decl> {',' <arg_decl>}*] ')'[':' <type>] <compound_statement>
		// Note about [':' <type>], if it isn't present, the function returns void
	*/

	// "func"
	if (matchKeyword(KeywordType::KW_FUNC))
	{
		auto rtr = std::make_unique<ASTFunctionDecl>();
		// <id>
		if (auto id = matchID())
			rtr->setDeclName(id);
		else
		{
			errorExpected("Expected an identifier");
			return ParsingResult<ASTFunctionDecl*>(false);
		}

		// '('
		if (!matchSign(SignType::S_ROUND_OPEN))
		{
			errorExpected("Expected '('");
			// try to resync to a ) without consuming it.
			if(!resyncToSignInFunction(SignType::S_ROUND_CLOSE,false))
				return ParsingResult<ASTFunctionDecl*>(false);
		}

		// [<arg_decl> {',' <arg_decl>}*]
		if (auto firstarg_res = parseArgDecl())
		{
			// Note, here, in the 2 places I've marked with (1) and (2), we can possibly
			// add error management, however, I don't think that's necessary since
			// the matchSign below will attempt to "panic and recover" if it doesn't find the )
			rtr->addArg(std::move(firstarg_res.result));
			while (true)
			{
				if (matchSign(SignType::S_COMMA))
				{
					if (auto argdecl_res = parseArgDecl())
						rtr->addArg(std::move(argdecl_res.result));
					else
					{
						if (argdecl_res.wasSuccessful()) // not found?
							errorExpected("Expected an argument declaration");
						// (1)
					}
				}
				else
					break;
			}
		}
		// (2)

		// ')'
		if (!matchSign(SignType::S_ROUND_CLOSE))
		{
			errorExpected("Expected a ')'");
			if (!resyncToSignInFunction(SignType::S_ROUND_CLOSE))
				return ParsingResult<ASTFunctionDecl*>(false);
		}
	
		// [':' <type>]
		if (matchSign(SignType::S_COLON))
		{
			auto rtrTy = parseType();
			if (rtrTy.first)
				rtr->setReturnType(rtrTy.first);
			else // no type found? we expected one after the colon!
			{
				if(rtrTy.second)
					errorExpected("Expected a type keyword");
				rtr->setReturnType(astcontext_.getPrimitiveVoidType());
				// don't return just yet, wait to see if a { can be found so we can still return something.
				// return ParsingResult<ASTFunctionDecl*>(false);
			}
		}
		else // if no return type, the function returns void.
			rtr->setReturnType(astcontext_.getPrimitiveVoidType());

		// Create recovery "enabling" object, since recovery is allowed for function bodies
		auto lock = createRecoveryEnabler();

		// <compound_statement>
		if (auto compstmt_res = parseCompoundStatement(/* mandatory = yes */ true))
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

ParsingResult<ASTArgDecl*> Parser::parseArgDecl()
{
	// <arg_decl> = <id> <fq_type_spec>
	// <id>
	if (auto id = matchID())
	{
		// <fq_type_spec>
		if (auto typespec_res = parseFQTypeSpec())
			return ParsingResult<ASTArgDecl*>(
					std::make_unique<ASTArgDecl>(id,typespec_res.result)
				);
		else
		{
			if(typespec_res.wasSuccessful())		// not found, report an error
				errorExpected("Expected a ':'");
			// in both case (not found or error) return an error
			return ParsingResult<ASTArgDecl*>(false);
		}
	}
	return ParsingResult<ASTArgDecl*>();
}

ParsingResult<ASTVarDecl*> Parser::parseVarDeclStmt()
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
			return ParsingResult<ASTVarDecl*>(
					resyncToSignInStatement(SignType::S_SEMICOLON) // Attempt to recover. If the recovery happened, the ParsingResult will just report a "not found" to let parsing continue, if the recovery
														// did not happend or the parser died, it'll report a failure.
				);
			// Note : we do not try to continue even if recovery was successful, as not enough information was gathered to return a valid node.
		}

		// <fq_type_spec>
		if (auto typespecResult = parseFQTypeSpec())
		{
			QualType ty = typespecResult.result;
			if (ty.isAReference())
			{
				context_.reportWarning("Ignored reference qualifier '&' in variable declaration : Variables cannot be references.");
				ty.setIsReference(false);
			}
			rtr->setType(ty);
		}
		else
		{
			errorExpected("Expected a ':'");
			if (resyncToSignInStatement(SignType::S_SEMICOLON))
				#pragma message("Here, change the return notfound to return a ASTParserRecovery node if it resynced successfully .");
			return ParsingResult<ASTVarDecl*>(
					resyncToSignInStatement(SignType::S_SEMICOLON) // See comment above, lines 162,163,165
				);
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
				// Recover to semicolon, return if recovery wasn't successful 
				if (!resyncToSignInStatement(SignType::S_SEMICOLON, /* do not consume the semi, so it can be picked up below */false))
					return ParsingResult<ASTVarDecl*>(false);
			}
		}

		// ';'
		if (!matchSign(SignType::S_SEMICOLON))
		{
			errorExpected("Expected ';'");
			
			// Try recovery if allowed. 
			if(!resyncToSignInStatement(SignType::S_SEMICOLON))
				return ParsingResult<ASTVarDecl*>(false);
			// else, recovery was successful, let the function return normally below.
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
		auto type = parseType();
		if (type.first)
			ty.setType(type.first);
		else
		{
			if(type.second) // if not found, return an error from us
				errorExpected("Expected a type");
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
	if (auto vdecl = parseVarDeclStmt()) // we don't recover on error because recovery is handled by parseUnit.
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