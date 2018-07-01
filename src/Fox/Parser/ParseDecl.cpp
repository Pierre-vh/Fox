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
#include "Fox/AST/ASTContext.hpp"

using namespace fox;


UnitDecl* Parser::parseUnit(const FileID& fid, IdentifierInfo* unitName, const bool& isMainUnit)
{
	// <fox_unit>	= {<declaration>}1+

	// Assert that unitName != nullptr
	assert(unitName && "Unit name cannot be nullptr!");
	assert(fid && "FileID cannot be invalid!");

	// Create the unit
	auto unit = std::make_unique<UnitDecl>(unitName, fid);

	// Create a RAIIDeclRecorder
	RAIIDeclRecorder raiidr(*this, unit.get());

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
				{
					// Report the error with the current token being the error location
					Token curtok = getCurtok();
					assert(curtok && "Curtok must be valid since we have not reached eof");
					diags_.report(DiagID::parser_expected_decl, curtok.getRange());
				}

				if (resyncToNextDecl())
					continue;
				else
					break;
			}
		}

	}

	// The conversion to uint16_t is needed so replacePlaceholder doesn't mistake
	// it for a character.
	if (std::uint16_t count = state_.curlyBracketsCount)
		diags_.report(DiagID::parser_missing_curlybracket, SourceLoc(fid)).addArg(count);

	if (std::uint16_t count = state_.roundBracketsCount)
		diags_.report(DiagID::parser_missing_roundbracket, SourceLoc(fid)).addArg(count);

	if (std::uint16_t count = state_.squareBracketsCount)
		diags_.report(DiagID::parser_missing_squarebracket, SourceLoc(fid)).addArg(count);

	if (unit->getDeclCount() == 0)
	{
		diags_.report(DiagID::parser_expected_decl_in_unit,SourceLoc(fid));
		return nullptr;
	}
	else
	{
		assert(unit->isValid());
		return astContext_.addUnit(std::move(unit), isMainUnit);
	}
}

Parser::DeclResult Parser::parseFunctionDecl()
{
	/*
		<func_decl>		= "func" <id> '(' [<arg_decl> {',' <arg_decl>}*] ')'[':' <type>] <compound_statement>
		// Note about [':' <type>], if it isn't present, the function returns void
	*/

	// To-Do :
		// Improve the error recovery on a missing '(' or ')' 

	// "func"
	auto fnKw = consumeKeyword(KeywordType::KW_FUNC);
	if (!fnKw)
		return DeclResult::NotFound();

	auto rtr = std::make_unique<FunctionDecl>(
		);
	SourceLoc begLoc = fnKw.getBeginSourceLoc();
	SourceLoc endLoc;

	bool isValid = true;
	bool foundLeftRoundBracket = true;

	// <id>
	if (auto foundID = consumeIdentifier())
		rtr->setIdentifier(foundID.get());
	else
	{
		reportErrorExpected(DiagID::parser_expected_iden);
		isValid = false;
		rtr->setIdentifier(identifiers_.getInvalidID());
	}

	// Before creating a RAIIDeclRecorder, record this function in the parent DeclRecorder
	if(isValid)
		recordDecl(rtr.get());

	// Create a RAIIDeclRecorder to record every decl within this function
	RAIIDeclRecorder raiidr(*this, rtr.get());

	// '('
	if (!consumeBracket(SignType::S_ROUND_OPEN))
	{
		reportErrorExpected(DiagID::parser_expected_opening_roundbracket);
		isValid = foundLeftRoundBracket = false;
	}

	// [<arg_decl> {',' <arg_decl>}*]
	if (auto firstarg = parseArgDecl())
	{
		rtr->addArg(firstarg.moveAs<ArgDecl>());
		while (true)
		{
			if (consumeSign(SignType::S_COMMA))
			{
				if (auto arg = parseArgDecl())
					rtr->addArg(arg.moveAs<ArgDecl>());
				else if(arg.wasSuccessful()) // not found?
					reportErrorExpected(DiagID::parser_expected_argdecl);
			}
			else
				break;
		}
	}

	// ')'
	if (auto rightParens = consumeBracket(SignType::S_ROUND_CLOSE))
		endLoc = rightParens;
	else 
	{
		isValid = false;
		reportErrorExpected(DiagID::parser_expected_closing_roundbracket);

		if (foundLeftRoundBracket) // Only attempt to recover if we found the ( before
		{
			// We'll attempt to recover to the '{' too, so if we find the body of the function
			// we can at least parse that.
			if (!resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ true))
				return DeclResult::Error();
		}
	}
	
	// [':' <type>]
	if (auto colon = consumeSign(SignType::S_COLON))
	{
		if (auto rtrTy = parseType())
		{
			rtr->setReturnType(rtrTy.get());
			endLoc = rtrTy.getSourceRange().makeEndSourceLoc();
		}
		else 
		{
			isValid = false;

			if (rtrTy.wasSuccessful())
				reportErrorExpected(DiagID::parser_expected_type);

			if (!resyncToSign(SignType::S_CURLY_OPEN, true, false))
				return DeclResult::Error();
		}
	}
	else // if no return type, the function returns void.
		rtr->setReturnType(astContext_.getPrimitiveVoidType());

	// <compound_statement>
	auto compoundstmt = parseCompoundStatement(/* mandatory = yes */ true);

	if (!compoundstmt || !isValid)
		return DeclResult::Error();

	rtr->setBody(compoundstmt.moveAs<CompoundStmt>());
	rtr->setSourceLocs(begLoc, endLoc, rtr->getBody()->getEndLoc());
	assert(rtr->isValid());
	return DeclResult(std::move(rtr));
}

Parser::DeclResult Parser::parseArgDecl()
{
	// <arg_decl> = <id> ':' <qualtype>

	// <id>
	auto id = consumeIdentifier();
	if (!id)
		return DeclResult::NotFound();

	// ':'
	if (!consumeSign(SignType::S_COLON))
	{
		reportErrorExpected(DiagID::parser_expected_colon);
		return DeclResult::Error();
	}

	// <qualtype>
	auto qt = parseQualType();
	if (!qt)
	{
		if (qt.wasSuccessful())
			reportErrorExpected(DiagID::parser_expected_type);
		return DeclResult::Error();
	}

	SourceLoc begLoc = id.getSourceRange().getBeginSourceLoc();
	SourceLoc endLoc = qt.getSourceRange().makeEndSourceLoc();
	auto rtr = std::make_unique<ArgDecl>(
			id.get(),
			qt.get(),
			begLoc,
			qt.getSourceRange(),
			endLoc
		);
	assert(rtr->isValid());
	recordDecl(rtr.get());
	return DeclResult(std::move(rtr));
}

Parser::DeclResult Parser::parseVarDecl()
{
	// <var_decl> = "let" <id> ':' <qualtype> ['=' <expr>] ';'
	// "let"
	auto letKw = consumeKeyword(KeywordType::KW_LET);
	if (!letKw)
		return DeclResult::NotFound();
	
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
		reportErrorExpected(DiagID::parser_expected_iden);
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
		reportErrorExpected(DiagID::parser_expected_colon);
		return DeclResult::Error();
	}

	// <qualtype>
	if (auto typespecResult = parseQualType())
	{
		ty = typespecResult.get();
		tyRange = typespecResult.getSourceRange();
		if (ty.isReference())
		{
			diags_.report(DiagID::parser_ignored_ref_vardecl, typespecResult.getSourceRange());
			ty.setIsReference(false);
		}
	}
	else
	{
		if (typespecResult.wasSuccessful())
			reportErrorExpected(DiagID::parser_expected_type);
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
			if (expr.wasSuccessful())
				reportErrorExpected(DiagID::parser_expected_expr);
			// Recover to semicolon, return if recovery wasn't successful 
			if (!resyncToSign(SignType::S_SEMICOLON, /*stopAtSemi (true/false doesn't matter when we're looking for a semi)*/ false, /*consumeToken*/ false))
				return DeclResult::Error();
		}
	}

	// ';'
	endLoc = consumeSign(SignType::S_SEMICOLON);
	if (!endLoc)
	{
		reportErrorExpected(DiagID::parser_expected_semi);
			
		if (!resyncToSign(SignType::S_SEMICOLON, /*stopAtSemi*/ false, /*consumeToken*/ false))
			return DeclResult::Error();

		endLoc = consumeSign(SignType::S_SEMICOLON);
	}

	auto rtr = std::make_unique<VarDecl>(id, ty, std::move(iExpr), begLoc,tyRange,endLoc);
	assert(rtr->isValid());
		
	// Record the decl
	recordDecl(rtr.get());

	return DeclResult(std::move(rtr));
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
		ty.setIsConst(true);
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
				reportErrorExpected(DiagID::parser_expected_type);
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