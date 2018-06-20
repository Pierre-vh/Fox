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
#include "Moonshot/Fox/AST/Decl.hpp"
#include "Moonshot/Fox/AST/Stmt.hpp"

#include <cassert>

using namespace Moonshot;


Parser::UnitResult Parser::parseUnit(const FileID& fid, IdentifierInfo* unitName)
{
	// <fox_unit>	= {<declaration>}1+

	// Assert that unitName != nullptr
	assert(unitName && "Unit name cannot be nullptr!");

	// Create the unit
	auto unit = std::make_unique<UnitDecl>(unitName, fid);

	// Create a RAIIDeclRecorder
	RAIIDeclRecorder raiidr(*this, unit.get());

	// Create recovery enabler.
	auto enabler = createRecoveryEnabler();

	// Parse declarations 
	while (true)
	{
		if (auto decl = parseDecl())
		{
			unit->addDecl(decl.move());
			continue;
		}
		else
		{
			// EOF/Died -> Break.
			if (isDone())
				break;
			// No EOF? There's an unexpected token on the way that prevents us from finding the decl.
			else
			{
				// Report an error in case of "not found";
				if (decl.wasSuccessful())
					errorExpected("Expected a declaration");

				if (resyncToNextDecl())
					continue;
				else
					break;
			}
		}

	}

	if (state_.curlyBracketsCount)
		genericError(std::to_string(state_.curlyBracketsCount) + " '}' still missing after parsing this unit.");

	if (state_.roundBracketsCount)
		genericError(std::to_string(state_.roundBracketsCount) + " ')' still missing after parsing this unit.");

	if (state_.squareBracketsCount)
		genericError(std::to_string(state_.squareBracketsCount) + " ']' still missing after parsing this unit.");

	if (unit->getDeclCount() == 0)
	{
		genericError("Expected one or more declaration in unit.");
		return UnitResult::Error();
	}
	else
		return UnitResult(std::move(unit));
}

Parser::DeclResult Parser::parseFunctionDecl()
{
	/*
		<func_decl>		= "func" <id> '(' [<arg_decl> {',' <arg_decl>}*] ')'[':' <type>] <compound_statement>
		// Note about [':' <type>], if it isn't present, the function returns void
	*/

	// "func"
	if (auto fnKw = consumeKeyword(KeywordType::KW_FUNC))
	{
		auto rtr = std::make_unique<FunctionDecl>(
			);
		SourceLoc begLoc = fnKw.getBeginSourceLoc();
		SourceLoc endLoc;

		bool isValid = true;
		// <id>
		if (auto foundID = consumeIdentifier())
			rtr->setIdentifier(foundID.get());
		else
		{
			errorExpected("Expected an identifier");
			isValid = false;
			rtr->setIdentifier(IDs.getInvalidID());
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
			rtr->addArg(firstarg.moveAs<ArgDecl>());
			while (true)
			{
				if (consumeSign(SignType::S_COMMA))
				{
					if (auto arg = parseArgDecl())
						rtr->addArg(arg.moveAs<ArgDecl>());
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
		if (auto rightParens = consumeBracket(SignType::S_ROUND_CLOSE))
			endLoc = rightParens;
		else 
		{
			errorExpected("Expected a ')'");
			if (!resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ false, /*consumeToken*/ true))
				return DeclResult::Error();
		}
	
		// [':' <type>]
		if (auto colon = consumeSign(SignType::S_COLON))
		{
			if (auto rtrTy = parseType())
			{
				rtr->setReturnType(rtrTy.get());
				endLoc = rtrTy.getSourceRange().makeEndSourceLoc();
			}
			else // no type found? we expected one after the colon!
			{
				if (rtrTy.wasSuccessful())
					errorExpected("Expected a type keyword");

				rtr->setReturnType(astcontext_.getPrimitiveVoidType());
				endLoc = colon;

				// Try to resync to a { so we can keep on parsing.
				if (!resyncToSign(SignType::S_CURLY_OPEN, false, false))
					return DeclResult::Error();
			}
		}
		else // if no return type, the function returns void.
			rtr->setReturnType(astcontext_.getPrimitiveVoidType());

		// <compound_statement>
		auto compoundstmt = parseCompoundStatement(/* mandatory = yes */ true);
		if (!compoundstmt)
			return DeclResult::Error();

		rtr->setBody(compoundstmt.moveAs<CompoundStmt>());
		rtr->setSourceLocs(begLoc, endLoc, rtr->getBody()->getEndLoc());
		assert(rtr->isComplete());
		return DeclResult(std::move(rtr));
	}
	return DeclResult::NotFound();
}

Parser::DeclResult Parser::parseArgDecl()
{
	// <arg_decl> = <id> ':' <qualtype>
	// <id>
	if (auto id = consumeIdentifier())
	{
		// ':'
		if (!consumeSign(SignType::S_COLON))
		{
			errorExpected("Expected a ':'");
			return DeclResult::Error();
		}

		// <qualtype>
		if (auto qt = parseQualType())
		{
			SourceLoc begLoc = id.getSourceRange().getBeginSourceLoc();
			SourceLoc endLoc = qt.getSourceRange().makeEndSourceLoc();
			auto rtr = std::make_unique<ArgDecl>(id.get(), qt.get(), begLoc, endLoc);
			recordDecl(rtr.get());
			return DeclResult(std::move(rtr));
		}
		else
		{
			if (qt.wasSuccessful())
				errorExpected("Expected a type");
			return DeclResult::Error();
		}
	}
	return DeclResult::NotFound();
}

Parser::DeclResult Parser::parseVarDecl()
{
	// <var_decl> = "let" <id> ':' <qualtype> ['=' <expr>] ';'
	// "let"
	if (auto letKw = consumeKeyword(KeywordType::KW_LET))
	{
		SourceLoc begLoc = letKw.getBeginSourceLoc();
		SourceLoc endLoc;
		SourceRange tyRange;

		IdentifierInfo* id;
		QualType ty;
		std::unique_ptr<Expr> iExpr;

		// <id>
		if (auto foundID = consumeIdentifier())
			id = foundID.get();
		else
		{
			errorExpected("Expected an identifier");
			if (auto res = resyncToSign(SignType::S_SEMICOLON, /* stopAtSemi (true/false doesn't matter when we're looking for a semi) */ false, /*consumeToken*/ true))
			{
				// Recovered? Act like nothing happened.
				return DeclResult::NotFound();
			}
			return DeclResult::Error();
		}

		// ':'
		if (!consumeSign(SignType::S_COLON))
		{
			errorExpected("Expected a ':'");
			return DeclResult::Error();
		}

		// <qualtype>
		if (auto typespecResult = parseQualType())
		{
			ty = typespecResult.get();
			tyRange = typespecResult.getSourceRange();
			if (ty.isAReference())
			{
				context_.reportWarning("Ignored reference qualifier '&' in variable declaration : Variables cannot be references.");
				ty.setIsReference(false);
			}
		}
		else
		{
			if(typespecResult.wasSuccessful())
				errorExpected("Expected a type");
			if (auto res = resyncToSign(SignType::S_SEMICOLON, /*stopAtSemi*/ true, /*consumeToken*/ true))
				return DeclResult::NotFound(); // Recovered? Act like nothing happened.
			return DeclResult::Error();
		}

		// ['=' <expr>]
		if (consumeSign(SignType::S_EQUAL))
		{
			if (auto expr = parseExpr())
				iExpr = expr.move();
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
		endLoc = consumeSign(SignType::S_SEMICOLON);
		if (!endLoc)
		{
			errorExpected("Expected ';'");
			
			if (!resyncToSign(SignType::S_SEMICOLON, /*stopAtSemi (true/false doesn't matter when we're looking for a semi)*/ false, /*consumeToken*/ true))
				return DeclResult::Error();
		}

		auto rtr = std::make_unique<VarDecl>(id, ty, std::move(iExpr), begLoc,tyRange,endLoc);
		assert(rtr->isComplete() && "Declaration isn't complete but parsing function completed successfully?");
		
		// Record the decl
		recordDecl(rtr.get());

		return DeclResult(std::move(rtr));
	}
	return DeclResult::NotFound();
}

Parser::Result<QualType> Parser::parseQualType()
{
	// 	<qualtype>	= ["const"] ['&'] <type>
	QualType ty;
	bool hasFoundSomething = false;
	SourceLoc begLoc;
	SourceLoc endLoc;
	// ["const"]
	if (auto kw = consumeKeyword(KeywordType::KW_CONST))
	{
		begLoc = kw.getBeginSourceLoc();
		hasFoundSomething = true;
		ty.setConstAttribute(true);
	}

	// ['&']
	if (auto ampersand = consumeSign(SignType::S_AMPERSAND))
	{
		// If no begLoc, the begLoc is the ampersand.
		if (!begLoc)
			begLoc = ampersand;
		hasFoundSomething = true;
		ty.setIsReference(true);
	}

	// <type>
	if (auto type = parseType())
	{
		ty.setType(type.get());

		// If no begLoc, the begLoc is the type's begLoc.
		if (!begLoc)
			begLoc = type.getSourceRange().getBeginSourceLoc();

		endLoc = type.getSourceRange().makeEndSourceLoc();
	}
	else
	{
		if (hasFoundSomething)
		{
			if (type.wasSuccessful())
				errorExpected("Expected a type");
			return Result<QualType>::Error();
		}
		else 
			return Result<QualType>::NotFound();
	}

	// Success!
	assert(ty && "Type cannot be null");
	assert(begLoc && "begLoc must be valid");
	assert(endLoc && "endLoc must be valid");
	return Result<QualType>(ty,SourceRange(begLoc,endLoc));

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