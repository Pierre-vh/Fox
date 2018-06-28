////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Parser.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
//	This file implements methods that aren't tied to Expression,
//	Statements or Declarations.
////------------------------------------------------------////

#include "Parser.hpp"

#include <sstream>
#include <cassert>
#include <iostream>
#include "Fox/AST/Identifiers.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/Common/SourceManager.hpp"

using namespace fox;

Parser::Parser(DiagnosticEngine& diags, SourceManager &sm, ASTContext& astctxt, TokenVector& l, DeclRecorder *dr) 
	: astContext_(astctxt), tokens_(l), identifiers_(astContext_.identifiers), srcMgr_(sm), diags_(diags)
{
	if (dr)
		state_.declRecorder = dr;

	setupParser();
}

ASTContext & Parser::getASTContext()
{
	return astContext_;
}

SourceManager& Parser::getSourceManager()
{
	return srcMgr_;
}

DiagnosticEngine& Parser::getDiagnosticEngine()
{
	return diags_;
}

void Parser::setupParser()
{
	// Setup iterators
	state_.tokenIterator = tokens_.begin();
	state_.lastUnexpectedTokenIt = tokens_.begin();
}

Parser::Result<IdentifierInfo*> Parser::consumeIdentifier()
{
	Token tok = getCurtok();
	if (tok.isIdentifier())
	{
		IdentifierInfo* ptr = tok.getIdentifierInfo();
		assert(ptr && "Token's an identifier but contains a nullptr IdentifierInfo?");
		incrementTokenIterator();
		return Result<IdentifierInfo*>(ptr,tok.getRange());
	}
	return Result<IdentifierInfo*>::NotFound();
}

SourceLoc Parser::consumeSign(const SignType & s)
{
	assert(!isBracket(s) && "This method shouldn't be used to match brackets ! Use consumeBracket instead!");
	auto tok = getCurtok();
	if (tok.is(s))
	{
		incrementTokenIterator();
		return tok.getRange().getBeginSourceLoc();
	}
	return SourceLoc();
}

SourceLoc Parser::consumeBracket(const SignType & s)
{
	assert(isBracket(s) && "This method should only be used on brackets ! Use consumeSign to match instead!");
	auto tok = getCurtok();
	if (tok.is(s))
	{
		switch (s)
		{
			case SignType::S_CURLY_OPEN:
				if (state_.curlyBracketsCount < maxBraceDepth_)
					state_.curlyBracketsCount++;
				else
					throw std::overflow_error("Max Brackets Depth Exceeded");
				break;
			case SignType::S_CURLY_CLOSE:
				if (state_.curlyBracketsCount)		// Don't let unbalanced parentheses create an underflow.
					state_.curlyBracketsCount--;
				break;
			case SignType::S_SQ_OPEN:
				if (state_.squareBracketsCount < maxBraceDepth_)
					state_.squareBracketsCount++;
				else
					throw std::overflow_error("Max Brackets Depth Exceeded");
				break;
			case SignType::S_SQ_CLOSE:
				if (state_.squareBracketsCount)		// Don't let unbalanced parentheses create an underflow.
					state_.squareBracketsCount--;
				break;
			case SignType::S_ROUND_OPEN:
				if (state_.roundBracketsCount < maxBraceDepth_)
					state_.roundBracketsCount++;
				else
					throw std::overflow_error("Max Brackets Depth Exceeded");
				break;
			case SignType::S_ROUND_CLOSE:
				if (state_.roundBracketsCount)		// Don't let unbalanced parentheses create an underflow.
					state_.roundBracketsCount--;
				break;
			default:
				throw std::exception("Unknown bracket type"); // Should be unreachable.
		}
		incrementTokenIterator();
		assert((tok.getRange().getOffset() == 0) && "Token is a sign but it's SourceRange offset is greater than zero?");
		return SourceLoc(tok.getRange().getBeginSourceLoc());
	}
	return SourceLoc();
}

SourceRange Parser::consumeKeyword(const KeywordType & k)
{
	auto tok = getCurtok();
	if (tok.is(k))
	{
		incrementTokenIterator();
		return tok.getRange();
	}
	return SourceRange();
}

bool Parser::peekNext(const SignType & s)
{
	return getCurtok().is(s);
}

bool Parser::peekNext(const KeywordType & s)
{
	return getCurtok().is(s);
}

void Parser::consumeAny()
{
	Token tok = getCurtok();
	// If it's a bracket, dispatch to consumeBracket. Else, just skip it.
	if (tok.isSign() && isBracket(tok.getSignType()))
		consumeBracket(tok.getSignType());
	else
		incrementTokenIterator();
}

void Parser::revertConsume()
{
	decrementTokenIterator();
	Token tok = getCurtok();

	if (isBracket(tok.getSignType()))
	{
		// Update bracket counters
		// We will be doing the exact opposite of what consumeBracket does !
		// That means : Decrementing the counter if a ( { [ is found, and incrementing it if a } ] ) is found.
		switch (tok.getSignType())
		{
			case SignType::S_CURLY_OPEN:
				if (state_.curlyBracketsCount)
					state_.curlyBracketsCount--;
				else
					throw std::overflow_error("Max Brackets Depth Exceeded");
				break;
			case SignType::S_CURLY_CLOSE:
				if (state_.curlyBracketsCount < maxBraceDepth_)
					state_.curlyBracketsCount++;
				break;
			case SignType::S_SQ_OPEN:
				if (state_.squareBracketsCount)
					state_.squareBracketsCount--;
				else
					throw std::overflow_error("Max Brackets Depth Exceeded");
				break;
			case SignType::S_SQ_CLOSE:
				if (state_.squareBracketsCount < maxBraceDepth_)
					state_.squareBracketsCount++;
				break;
			case SignType::S_ROUND_OPEN:
				if (state_.roundBracketsCount)
					state_.roundBracketsCount--;
				else
					throw std::overflow_error("Max Brackets Depth Exceeded");
				break;
			case SignType::S_ROUND_CLOSE:
				if (state_.roundBracketsCount < maxBraceDepth_)
					state_.roundBracketsCount++;
				break;
			default:
				throw std::exception("Unknown bracket type"); // Should be unreachable.
		}
	}
	// Else, we're done. For now, only brackets have counters associated with them.
}

void Parser::incrementTokenIterator()
{
	if (state_.tokenIterator != tokens_.end())
		state_.tokenIterator++;
}

void Parser::decrementTokenIterator()
{
	if (state_.tokenIterator != tokens_.begin())
		state_.tokenIterator--;
}

bool Parser::isBracket(const SignType & s) const
{
	switch (s)
	{
		case SignType::S_CURLY_OPEN:
		case SignType::S_CURLY_CLOSE:
		case SignType::S_SQ_OPEN:
		case SignType::S_SQ_CLOSE:
		case SignType::S_ROUND_OPEN:
		case SignType::S_ROUND_CLOSE:
			return true;
		default:
			return false;
	}
}

Parser::Result<Type*> Parser::parseBuiltinTypename()
{
	// <builtin_type_name> 	= "int" | "float" | "bool" | "string" | "char"

	typedef Parser::Result<Type*> RtrTy;

	// "int"
	if (auto range = consumeKeyword(KeywordType::KW_INT))
		return RtrTy(astContext_.getPrimitiveIntType(),range);
	
	// "float"
	if (auto range = consumeKeyword(KeywordType::KW_FLOAT))
		return RtrTy(astContext_.getPrimitiveFloatType(), range);

	// "bool"
	if (auto range = consumeKeyword(KeywordType::KW_BOOL))
		return RtrTy(astContext_.getPrimitiveBoolType(), range);

	// "string"
	if (auto range = consumeKeyword(KeywordType::KW_STRING))
		return RtrTy(astContext_.getPrimitiveStringType(), range);

	// "char"
	if (auto range = consumeKeyword(KeywordType::KW_CHAR))
		return RtrTy(astContext_.getPrimitiveCharType(), range);

	return RtrTy::NotFound();
}

Parser::Result<Type*> Parser::parseType()
{
	// <type> = <builtin_type_name> { '[' ']' }
	// <builtin_type_name> 
	if (auto ty_res = parseBuiltinTypename())
	{
		//  { '[' ']' }
		Type* ty = ty_res.get();
		SourceLoc begLoc = ty_res.getSourceRange().getBeginSourceLoc();
		SourceLoc endLoc = ty_res.getSourceRange().makeEndSourceLoc();
		while (consumeBracket(SignType::S_SQ_OPEN))
		{
			ty = astContext_.getArrayTypeForType(ty);
			// ']'
			if (auto right = consumeBracket(SignType::S_SQ_CLOSE))
				endLoc = right;
			else
			{
				reportErrorExpected(DiagID::parser_expected_closing_squarebracket);

				if (resyncToSign(SignType::S_SQ_CLOSE,/*stopAtSemi */ true,/*shouldConsumeToken*/ false))
				{
					endLoc = consumeBracket(SignType::S_SQ_CLOSE);
					continue;
				}

				return Result<Type*>::Error();
			}
		}
		return Result<Type*>(ty, SourceRange(begLoc, endLoc));
	}
	return Result<Type*>::NotFound();
}

Token Parser::getCurtok() const
{
	if (!isDone())
		return *state_.tokenIterator;
	return Token();
}

Token Parser::getPreviousToken() const
{
	auto it = state_.tokenIterator;
	if (it != tokens_.begin())
		return *(--it);
	return Token();
}

bool Parser::resyncToSign(const SignType & sign, const bool & stopAtSemi, const bool & shouldConsumeToken)
{
	return resyncToSign(std::vector<SignType>({ sign }), stopAtSemi, shouldConsumeToken);
}

bool Parser::resyncToSign(const std::vector<SignType>& signs, const bool & stopAtSemi, const bool & shouldConsumeToken)
{
	if (!isAlive())
		return false;

	bool isFirst = true;
	// Keep going until we reach EOF.
	while(!isDone())
	{
		// Check curtok
		auto tok = getCurtok();
		for (auto it = signs.begin(), end = signs.end(); it != end; it++)
		{
			if (tok.is(*it))
			{
				if (shouldConsumeToken)
					consumeAny();

				return true;
			}
		}

		if (tok.isSign())
		{
			switch (tok.getSignType())
			{
				// Skip nested brackets
				case SignType::S_CURLY_OPEN:
					consumeBracket(SignType::S_CURLY_OPEN);
					resyncToSign(SignType::S_CURLY_CLOSE, false, true);
					break;
				case SignType::S_SQ_OPEN:
					consumeBracket(SignType::S_SQ_OPEN);
					resyncToSign(SignType::S_SQ_CLOSE, false, true);
					break;
				case SignType::S_ROUND_OPEN:
					consumeBracket(SignType::S_ROUND_OPEN);
					resyncToSign(SignType::S_ROUND_CLOSE, false, true);
					break;
				// Skip closing brackets if they're unbalanced, else
				// return to avoid escaping the current block.
				case SignType::S_CURLY_CLOSE:
					if (state_.curlyBracketsCount && !isFirst)
						return false;
					consumeBracket(SignType::S_CURLY_CLOSE);
					break;
				case SignType::S_SQ_CLOSE:
					if (state_.squareBracketsCount && !isFirst)
						return false;
					consumeBracket(SignType::S_SQ_CLOSE);
					break;
				case SignType::S_ROUND_CLOSE:
					if (state_.roundBracketsCount && !isFirst)
						return false;
					consumeBracket(SignType::S_ROUND_CLOSE);
					break;
				case SignType::S_SEMICOLON:
					if (stopAtSemi)
						return false;
					// fallthrough
				default:
					consumeAny();
					break;
			}
		} // (tok.isSign())
		else 
			consumeAny();

		isFirst = false;
	}
	// If reached eof, die & return false.
	die();
	return false;
}

bool Parser::resyncToNextDecl()
{
	// This method skips everything until it finds a "let" or a "func".

	// Return immediately if recovery is not allowed, or the parser isn't alive anymore.
	if (!isAlive())
		return false;

	while(!isDone())
	{
		auto tok = getCurtok();
		// if it's let/func, return.
		if (tok.is(KeywordType::KW_FUNC) || tok.is(KeywordType::KW_LET))
			return true;
		// Check if it's a sign for special behaviours
		if (tok.isSign())
		{
			switch (tok.getSignType())
			{
					// If we find a '(', '{' or '[', call resyncToSign to skip to it's counterpart.
				case SignType::S_CURLY_OPEN:
					consumeBracket(SignType::S_CURLY_OPEN);
					resyncToSign(SignType::S_CURLY_CLOSE, false, true);
					break;
				case SignType::S_SQ_OPEN:
					consumeBracket(SignType::S_SQ_OPEN);
					resyncToSign(SignType::S_SQ_CLOSE, false, true);
					break;
				case SignType::S_ROUND_OPEN:
					consumeBracket(SignType::S_ROUND_OPEN);
					resyncToSign(SignType::S_ROUND_CLOSE, false, true);
					break;
					// If we find a ')', '}' or ']' we just consume it.
				case SignType::S_CURLY_CLOSE:
					consumeBracket(SignType::S_CURLY_CLOSE);
					break;
				case SignType::S_SQ_CLOSE:
					consumeBracket(SignType::S_SQ_CLOSE);
					break;
				case SignType::S_ROUND_CLOSE:
					consumeBracket(SignType::S_ROUND_CLOSE);
					break;
				default:
					consumeAny();
					break;
			}
		} // (tok.isSign())
		else 
			consumeAny();
	}
	// If reached eof, die & return false.
	die();
	return false;
}

void Parser::die()
{
	state_.tokenIterator = tokens_.end();
	state_.isAlive = false;
}

void Parser::recordDecl(NamedDecl * nameddecl)
{
	// Only assert when we're not in test mode.
	// Tests may call individual parsing function, and won't care about if a DeclRecorder is active or not.
	assert(state_.declRecorder && "Decl Recorder cannot be null when parsing a Declaration!");
	if(state_.declRecorder)
		state_.declRecorder->recordDecl(nameddecl);
}

Diagnostic Parser::reportErrorExpected(const DiagID& diag)
{
	SourceRange errorRange;
	if (Token prevTok = getPreviousToken())
	{
		SourceLoc loc = prevTok.getRange().makeEndSourceLoc();
		loc.increment();
		assert(srcMgr_.isSourceLocValid(loc));
		errorRange = SourceRange(loc);
	}
	else
	{
		// No valid previous token, use the current token's range as the 
		// error location. (This case should be fairly rare, or never happen at all. tests needed)
		Token curTok = getCurtok();
		assert(curTok && "No valid previous token and no valid current token?");
		errorRange = curTok.getRange();
	}
	return diags_.report(diag, errorRange);
}

bool Parser::isDone() const
{
	return (state_.tokenIterator == tokens_.end()) || (!isAlive());
}

bool Parser::isAlive() const
{
	return state_.isAlive;
}

Parser::ParserState Parser::createParserStateBackup() const
{
	return state_;
}

void Parser::restoreParserStateFromBackup(const Parser::ParserState & st)
{
	state_ = st;
}

// ParserState
Parser::ParserState::ParserState() : isAlive(true), isRecoveryAllowed(false)
{

}

// RAIIDeclRecorder
Parser::RAIIDeclRecorder::RAIIDeclRecorder(Parser &p, DeclRecorder *dr) : parser_(p)
{
	declRec_ = parser_.state_.declRecorder;

	// If declRec_ isn't null, mark it as the parent of the new dr
	if (declRec_)
	{
		// Assert that we're not overwriting a parent. If such a thing happens, that could indicate a bug!
		assert(!dr->hasParentDeclRecorder() && "New DeclRecorder already has a parent?");
		dr->setParentDeclRecorder(declRec_);
	}

	parser_.state_.declRecorder = dr;
}

Parser::RAIIDeclRecorder::~RAIIDeclRecorder()
{
	parser_.state_.declRecorder = declRec_;
}