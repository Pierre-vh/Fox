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

#include "Fox/Parser/Parser.hpp"
#include "Fox/AST/ASTContext.hpp"

using namespace fox;


UnitDecl* Parser::parseUnit(const FileID& fid, Identifier* unitName, const bool& isMainUnit)
{
	// <fox_unit>	= {<declaration>}1+

	// Assert that unitName != nullptr
	assert(unitName && "Unit name cannot be nullptr!");
	assert(fid && "FileID cannot be invalid!");

	// Create the unit
	auto* unit = new(ctxt_) UnitDecl(unitName, fid);

	// Create a RAIIDeclContext
	RAIIDeclContext raiidr(*this, unit);

	bool declHadError = false;

	// Parse declarations 
	while (true)
	{
		if (auto decl = parseDecl())
		{
			unit->addDecl(decl.get());
			continue;
		}
		else
		{
			if (!decl.wasSuccessful())
				declHadError = true;

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

	if (unit->getDeclCount() == 0)
	{
		if(!declHadError)
			diags_.report(DiagID::parser_expected_decl_in_unit, fid);
		return nullptr;
	}
	else
	{
		assert(unit->isValid());
		ctxt_.addUnit(unit, isMainUnit);
		return unit;
	}
}

Parser::DeclResult Parser::parseFuncDecl()
{
	/*
		<func_decl>	= "func" <id> '(' [<param_decl> {',' <param_decl>}*] ')'[':' <type>] <compound_statement>
		// Note about [':' <type>], if it isn't present, the function returns void
	*/

	// FIXME:
		// Improve the error recovery on a missing '(' or ')' 

	// "func"
	auto fnKw = consumeKeyword(KeywordType::KW_FUNC);
	if (!fnKw)
		return DeclResult::NotFound();

	// The return node
	FuncDecl* rtr = new(ctxt_) FuncDecl();

	// Locs
	SourceLoc begLoc = fnKw.getBegin();
	SourceLoc headEndLoc;

	// Poisoned is set to true if the 
	// declarations is missing stuff (such as the ID)
	// If poisoned = true, we won't push the decl and
	// we will return an error after parsing.
	bool poisoned = false;

	// <id>
	if (auto foundID = consumeIdentifier())
	{
		rtr->setIdentifier(foundID.get());
		// Before creating a RAIIDeclContext, record this function in the parent DeclContext.
		// We only record the function if it's valid!
		recordDecl(rtr);
	}
	else
	{
		reportErrorExpected(DiagID::parser_expected_iden);
		//rtr->setIdentifier(identifiers_.getInvalidID());
		poisoned = true;
	}


	// Create a RAIIDeclContext to record every decl within this function
	RAIIDeclContext raiiDC(*this, rtr);

	// '('
	if (!consumeBracket(SignType::S_ROUND_OPEN))
	{
		// IDEA:: Instead of giving up immediately, maybe we could try to
		// parse more? Would it be useful? For now, I don't know.
		// Time will tell.
		if (poisoned)
			return DeclResult::Error();

		reportErrorExpected(DiagID::parser_expected_opening_roundbracket);

	}

	// [<param_decl> {',' <param_decl>}*]
	if (auto first = parseParamDecl())
	{
		rtr->addParam(first.getAs<ParamDecl>());
		while (true)
		{
			if (consumeSign(SignType::S_COMMA))
			{
				if (auto param = parseParamDecl())
					rtr->addParam(param.getAs<ParamDecl>());
				else if (param.wasSuccessful())
				{
					// IDEA: Maybe reporting the error after the "," would yield
					// better error messages?
					reportErrorExpected(DiagID::parser_expected_argdecl);
				}
			}
			else
				break;
		}
	}

	// ')'
	if (auto rightParens = consumeBracket(SignType::S_ROUND_CLOSE))
		headEndLoc = rightParens;
	else 
	{
		reportErrorExpected(DiagID::parser_expected_closing_roundbracket);

		// We'll attempt to recover to the '{' too, so if we find the body of the function
		// we can at least parse that.
		if (!resyncToSign(SignType::S_ROUND_CLOSE, /* stopAtSemi */ true, /*consumeToken*/ false))
			return DeclResult::Error();

		headEndLoc = consumeBracket(SignType::S_ROUND_CLOSE);
	}
	
	// [':' <type>]
	if (auto colon = consumeSign(SignType::S_COLON))
	{
		if (auto rtrTy = parseType())
		{
			TypeLoc tl = rtrTy.getAsTypeLoc();
			rtr->setReturnType(tl);
			headEndLoc = tl.getRange().getEnd();
		}
		else 
		{
			if (rtrTy.wasSuccessful())
				reportErrorExpected(DiagID::parser_expected_type);

			if (!resyncToSign(SignType::S_CURLY_OPEN, true, false))
				return DeclResult::Error();
			// If resynced successfully, use the colon as the end of the header
			// and consider the return type to be void
			headEndLoc = colon;
			rtr->setReturnType(ctxt_.getVoidType());
		}
	}
	else // if no return type, the function returns void.
		rtr->setReturnType(ctxt_.getVoidType());

	// <compound_statement>
	StmtResult compStmt = parseCompoundStatement();

	if (!compStmt)
	{
		if(compStmt.wasSuccessful()) // Display only if it was not found
			reportErrorExpected(DiagID::parser_expected_opening_curlybracket);
		return DeclResult::Error();
	}

	auto* body = dyn_cast<CompoundStmt>(compStmt.get());
	assert(body && "Not a compound stmt");

	// Finished parsing, return unless the decl is poisoned.
	if (poisoned)
		return DeclResult::Error();

	SourceRange range(begLoc, body->getRange().getEnd());
	assert(headEndLoc && range && "Invalid loc info");

	rtr->setBody(body);
	rtr->setLocs(range, headEndLoc);
	assert(rtr->isValid() && "Decl should be valid at this stage");
	return DeclResult(rtr);
}

Parser::DeclResult Parser::parseParamDecl()
{
	// <param_decl> = <id> ':' <qualtype>

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
	auto typeResult = parseQualType();
	if (!typeResult)
	{
		if (typeResult.wasSuccessful())
			reportErrorExpected(DiagID::parser_expected_type);
		return DeclResult::Error();
	}

	TypeLoc tl(typeResult.get().type, typeResult.getRange());
	bool isConst = typeResult.get().isConst;

	SourceLoc begLoc = id.getRange().getBegin();
	SourceLoc endLoc = tl.getRange().getEnd();

	SourceRange range(begLoc, endLoc);

	assert(range && "Invalid loc info");

	auto* rtr = new(ctxt_) ParamDecl(
			id.get(),
			tl,
			isConst,
			range
		);
	assert(rtr->isValid());
	recordDecl(rtr);
	return DeclResult(rtr);
}

Parser::DeclResult Parser::parseVarDecl()
{
	// <var_decl> = "let" <id> ':' <qualtype> ['=' <expr>] ';'
	// "let"
	auto letKw = consumeKeyword(KeywordType::KW_LET);
	if (!letKw)
		return DeclResult::NotFound();
	
	SourceLoc begLoc = letKw.getBegin();
	SourceLoc endLoc;

	Identifier* id;
	TypeLoc type;
	bool isConst = false;
	Expr* iExpr = nullptr;

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
	SourceLoc ampLoc;
	if (auto qtRes = parseQualType(nullptr, &ampLoc))
	{
		type = TypeLoc(qtRes.get().type, qtRes.getRange());
		isConst = qtRes.get().isConst;
		if (qtRes.get().isRef)
			diags_.report(DiagID::parser_ignored_ref_vardecl, ampLoc);
	}
	else
	{
		if (qtRes.wasSuccessful())
			reportErrorExpected(DiagID::parser_expected_type);
		if (auto res = resyncToSign(SignType::S_SEMICOLON, /*stopAtSemi*/ true, /*consumeToken*/ true))
			return DeclResult::NotFound(); // Recovered? Act like nothing happened.
		return DeclResult::Error();
	}

	// ['=' <expr>]
	if (consumeSign(SignType::S_EQUAL))
	{
		if (auto expr = parseExpr())
			iExpr = expr.get();
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

	SourceRange range(begLoc, endLoc);
	assert(range && "Invalid loc info");
	assert(type && "type is not valid");
	assert(type.getRange() && "type range is not valid");
	auto rtr = new(ctxt_) VarDecl(id, type, isConst, iExpr, range);
	assert(rtr->isValid());
	recordDecl(rtr);
	return DeclResult(rtr);
}

Parser::Result<Parser::ParsedQualType> Parser::parseQualType(SourceRange* constRange, SourceLoc* refLoc)
{
	// 	<qualtype>	= ["const"] ['&'] <type>
	ParsedQualType rtr;
	bool hasFoundSomething = false;
	SourceLoc begLoc, endLoc;

	// ["const"]
	if (auto kw = consumeKeyword(KeywordType::KW_CONST))
	{
		begLoc = kw.getBegin();
		hasFoundSomething = true;
		rtr.isConst = true;

		if (constRange)
			(*constRange) = kw;
	}

	// ['&']
	if (auto ampersand = consumeSign(SignType::S_AMPERSAND))
	{
		// If no begLoc, the begLoc is the ampersand.
		if (!begLoc)
			begLoc = ampersand;
		hasFoundSomething = true;
		rtr.isRef = true;

		if (refLoc)
			(*refLoc) = ampersand;
	}

	// <type>
	if (auto tyRes = parseType())
	{
		TypeLoc tl = tyRes.getAsTypeLoc();
		rtr.type = tl; // convert to Type

		// If no begLoc, the begLoc is the type's begLoc.
		if (!begLoc)
			begLoc = tl.getRange().getBegin();

		endLoc = tl.getRange().getEnd();
	}
	else
	{
		if (hasFoundSomething)
		{
			if (tyRes.wasSuccessful())
				reportErrorExpected(DiagID::parser_expected_type);
			return Result<ParsedQualType>::Error();
		}
		else 
			return Result<ParsedQualType>::NotFound();
	}

	assert(rtr.type && "Type cannot be invalid");
	assert(begLoc && "begLoc must be valid");
	assert(endLoc && "endLoc must be valid");
	return Result<ParsedQualType>(rtr, SourceRange(begLoc,endLoc));
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
	if (auto fdecl = parseFuncDecl())
		return fdecl;
	else if (!fdecl.wasSuccessful())
		return DeclResult::Error();

	return DeclResult::NotFound();
}