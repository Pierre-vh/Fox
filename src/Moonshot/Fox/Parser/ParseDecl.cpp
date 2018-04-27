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

#include "Moonshot/Fox/Common/Context.hpp"
#include "Moonshot/Fox/AST/ASTContext.hpp"
#include "Moonshot/Fox/AST/ASTDecl.hpp"
#include "Moonshot/Fox/AST/ASTStmt.hpp"

#include <cassert>

using namespace Moonshot;

Parser::DeclResult Parser::parseFunctionDecl()
{
	/*
		<func_decl>		= "func" <id> '(' [<arg_decl> {',' <arg_decl>}*] ')'[':' <type>] <compound_statement>
		// Note about [':' <type>], if it isn't present, the function returns void
	*/

	// "func"
	if (consumeKeyword(KeywordType::KW_FUNC))
	{
		auto rtr = std::make_unique<ASTFunctionDecl>();

		bool isValid = true;
		// <id>
		if (auto id = consumeIdentifier())
			rtr->setIdentifier(id);
		else
		{
			errorExpected("Expected an identifier");
			isValid = false;
			// Here, continue parsing. This might generate an error cascade but we need to try and parse more things before giving up definitely.
		}

		// Before creating a RAIIDeclRecorder, record this function in the parent DeclRecorder
		if(isValid)
			recordDecl(rtr.get());
		// Create a RAIIDeclRecorder to record every decl that happens within this
		// function parsing.
		RAIIDeclRecorder raiidr(*this, rtr.get());

		// '('
		if (!consumeBracket(SignType::S_ROUND_OPEN))
		{
			errorExpected("Expected '('");
			// try to resync to a ) without consuming it.
			if (!resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ false))
				return DeclResult::Error();
		}

		// [<arg_decl> {',' <arg_decl>}*]
		if (auto firstarg = parseArgDecl())
		{
			// Note, here, in the 2 places I've marked with (1) and (2), we can possibly
			// add error management, however, I don't think that's necessary since
			// the consumeBracket below will attempt to "panic and recover" if it doesn't find the )
			// About (1), maybe a break could be added there, but I think it's just better to ignore and try to parse more.
			rtr->addArg(firstarg.moveAs<ASTArgDecl>());
			while (true)
			{
				if (consumeSign(SignType::S_COMMA))
				{
					if (auto arg = parseArgDecl())
						rtr->addArg(arg.moveAs<ASTArgDecl>());
					else
					{
						if (arg.wasSuccessful()) // not found?
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
		if (!consumeBracket(SignType::S_ROUND_CLOSE))
		{
			errorExpected("Expected a ')'");
			if (!resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ false, /*consumeToken*/ true))
				return DeclResult::Error();
		}

	
		// [':' <type>]
		if (consumeSign(SignType::S_COLON))
		{
			if (auto rtrTy = parseType())
				rtr->setReturnType(rtrTy.get());
			else // no type found? we expected one after the colon!
			{
				if (rtrTy.wasSuccessful())
					errorExpected("Expected a type keyword");
				rtr->setReturnType(astcontext_.getPrimitiveVoidType());

				// Try to resync to a { so we can keep on parsing.
				if (!resyncToSign(SignType::S_CURLY_OPEN, false, false))
					return DeclResult::Error();
			}
		}
		else // if no return type, the function returns void.
			rtr->setReturnType(astcontext_.getPrimitiveVoidType());

		// <compound_statement>
		if (auto compoundstmt = parseCompoundStatement(/* mandatory = yes */ true))
		{
			rtr->setBody(compoundstmt.moveAs<ASTCompoundStmt>());
			// Success, nothing more to see here!
			if (isValid)
			{
				assert(rtr->isValid() && "Declaration is invalid but parsing function completed successfully?");
				return DeclResult(std::move(rtr));
			}
		}
		return DeclResult::Error();
	}
	return DeclResult::NotFound();
}

Parser::DeclResult Parser::parseArgDecl()
{
	// <arg_decl> = <id> <fq_type_spec>
	// <id>
	if (auto id = consumeIdentifier())
	{
		// <fq_type_spec>
		if (auto typespec_res = parseFQTypeSpec())
		{
			auto rtr = std::make_unique<ASTArgDecl>(id, typespec_res.get());
			recordDecl(rtr.get());
			return DeclResult(std::move(rtr));
		}
		else
		{
			if(typespec_res.wasSuccessful())		
				errorExpected("Expected a ':'");
			return DeclResult::Error();
		}
	}
	return DeclResult::NotFound();
}

Parser::DeclResult Parser::parseVarDecl()
{
	// <var_decl> = "let" <id> <fq_type_spec> ['=' <expr>] ';'
	// "let"
	if (consumeKeyword(KeywordType::KW_LET))
	{
		auto rtr = std::make_unique<ASTVarDecl>();

		// <id>
		if (auto id = consumeIdentifier())
			rtr->setIdentifier(id);
		else
		{
			errorExpected("Expected an identifier");
			if (auto res = resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi (true/false doesn't matter when we're looking for a semi) */ false, /*consumeToken*/ true))
			{
				return DeclResult(
						std::make_unique<ASTVarDecl>()	// If we recovered, return an empty (invalid) var decl.
					);
				// Note : we don't record this decl, since it's invalid and can't be used.
			}
			return DeclResult::Error();
		}

		// <fq_type_spec>
		if (auto typespecResult = parseFQTypeSpec())
		{
			QualType ty = typespecResult.get();
			if (ty.isAReference())
			{
				context_.reportWarning("Ignored reference qualifier '&' in variable declaration : Variables cannot be references.");
				ty.setIsReference(false);
			}
			rtr->setType(ty);
		}
		else
		{
			if(typespecResult.wasSuccessful())
				errorExpected("Expected a ':'");
			if (auto res = resyncToSign(SignType::S_SEMICOLON, /*stopAtSemi (true/false doesn't matter when we're looking for a semi)*/ true, /*consumeToken*/ true))
			{
				return DeclResult(
						std::make_unique<ASTVarDecl>()	// If we recovered, return an empty (invalid) var decl.
					);
				// Note : we don't record this decl, since it's invalid and can't be used.
			}
			return DeclResult::Error();
		}

		// ['=' <expr>]
		if (consumeSign(SignType::S_EQUAL))
		{
			if (auto expr = parseExpr())
				rtr->setInitExpr(expr.move());
			else
			{
				if(expr.wasSuccessful())
					errorExpected("Expected an expression");
				// Recover to semicolon, return if recovery wasn't successful 
				if (!resyncToSign(SignType::S_SEMICOLON, /*stopAtSemi (true/false doesn't matter when we're looking for a semi)*/ false, /*consumeToken*/ false))
					return DeclResult::Error();
			}
		}

		// ';'
		if (!consumeSign(SignType::S_SEMICOLON))
		{
			errorExpected("Expected ';'");
			
			if (!resyncToSign(SignType::S_SEMICOLON, /*stopAtSemi (true/false doesn't matter when we're looking for a semi)*/ false, /*consumeToken*/ true))
				return DeclResult::Error();
		}
		// If we're here, assert that the node is valid.
		assert(rtr->isValid() && "Declaration is invalid but parsing function completed successfully?");
		// Record the decl
		recordDecl(rtr.get());
		// return
		return DeclResult(std::move(rtr));
	}
	// not found
	return DeclResult::NotFound();
}

Parser::Result<QualType> Parser::parseFQTypeSpec()
{
	// 	<fq_type_spec>	= ':' ["const"] ['&'] <type>
	if (consumeSign(SignType::S_COLON))
	{
		QualType ty;
		// ["const"]
		if (consumeKeyword(KeywordType::KW_CONST))
			ty.setConstAttribute(true);

		// ['&']
		if (consumeSign(SignType::S_AMPERSAND))
			ty.setIsReference(true);

		// <type>
		if (auto type = parseType())
			ty.setType(type.get());
		else
		{
			if(type.wasSuccessful()) 
				errorExpected("Expected a type");
			return Result<QualType>::Error();
		}

		// Success!
		return Result<QualType>(ty);
	}
	// not found!
	return Result<QualType>::NotFound();
}

Parser::DeclResult Parser::parseDecl()
{
	// <declaration> = <var_decl> | <func_decl>

	// <var_decl>
	if (auto vdecl = parseVarDecl())
		return vdecl;
	else if (!vdecl.wasSuccessful())
		return DeclResult::Error();

	// <func_decl>
	if (auto fdecl = parseFunctionDecl())
		return fdecl;
	else if (!fdecl.wasSuccessful())
		return DeclResult::Error();

	return DeclResult::NotFound();
}