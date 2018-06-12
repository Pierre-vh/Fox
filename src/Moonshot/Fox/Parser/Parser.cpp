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
#include "Moonshot/Fox/AST/Identifiers.hpp"
#include "Moonshot/Fox/Common/Context.hpp"
#include "Moonshot/Fox/Common/Exceptions.hpp"

using namespace Moonshot;

Parser::Parser(Context& c, ASTContext& astctxt, TokenVector& l,DeclRecorder *dr) : context_(c), astcontext_(astctxt), tokens_(l)
{
	if (dr)
		state_.declRecorder = dr;

	isTestMode_ = false;

	setupParser();
}

Parser::UnitResult Parser::parseUnit(const FileID& fid,IdentifierInfo* unitName)
{
	// <fox_unit>	= {<declaration>}1+

	// Assert that unitName != nullptr
	assert(unitName && "Unit name cannot be nullptr!");

	// Create the unit
	auto unit = std::make_unique<UnitDecl>(unitName,fid);

	// Create a RAIIDeclRecorder
	RAIIDeclRecorder raiidr(*this,unit.get());

	// Create recovery enabler.
	auto enabler = createRecoveryEnabler();

	// Gather some flags
	const bool showRecoveryMessages = context_.flagsManager.isSet(FlagID::parser_showRecoveryMessages);

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

				if (showRecoveryMessages)
					genericError("Attempting recovery to next declaration.");

				if (resyncToNextDecl())
				{
					if(showRecoveryMessages)
						genericError("Recovered successfully."); // Note : add a position, like "Recovered successfuly at line x"
					continue;
				}
				else
				{
					if(showRecoveryMessages)
						genericError("Couldn't recover.");
					break;
				}
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

void Parser::enableTestMode()
{
	isTestMode_ = true;
}

void Parser::disableTestMode()
{
	isTestMode_ = false;
}

ASTContext & Parser::getASTContext()
{
	return astcontext_;
}

Context & Parser::getContext()
{
	return context_;
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
		skipToken();
		return Result<IdentifierInfo*>(ptr,tok.sourceRange);
	}
	return Result<IdentifierInfo*>::NotFound();
}

SourceLoc Parser::consumeSign(const SignType & s)
{
	assert(!isBracket(s) && "This method shouldn't be used to match brackets ! Use consumeBracket instead!");
	auto tok = getCurtok();
	if (tok.is(s))
	{
		skipToken();
		return tok.sourceRange.getBeginSourceLoc();
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
				if (state_.curlyBracketsCount < kMaxBraceDepth)
					state_.curlyBracketsCount++;
				else
					throw std::overflow_error("Max Brackets Depth Exceeded");
				break;
			case SignType::S_CURLY_CLOSE:
				if (state_.curlyBracketsCount)		// Don't let unbalanced parentheses create an underflow.
					state_.curlyBracketsCount--;
				break;
			case SignType::S_SQ_OPEN:
				if (state_.squareBracketsCount < kMaxBraceDepth)
					state_.squareBracketsCount++;
				else
					throw std::overflow_error("Max Brackets Depth Exceeded");
				break;
			case SignType::S_SQ_CLOSE:
				if (state_.squareBracketsCount)		// Don't let unbalanced parentheses create an underflow.
					state_.squareBracketsCount--;
				break;
			case SignType::S_ROUND_OPEN:
				if (state_.roundBracketsCount < kMaxBraceDepth)
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
		skipToken();
		assert((tok.sourceRange.getOffset() == 0) && "Token is a sign but it's SourceRange offset is greater than zero?");
		return SourceLoc(tok.sourceRange.getBeginSourceLoc());
	}
	return SourceLoc();
}

SourceRange Parser::consumeKeyword(const KeywordType & k)
{
	auto tok = getCurtok();
	if (tok.is(k))
	{
		skipToken();
		return tok.sourceRange;
	}
	return SourceRange();
}

void Parser::consumeAny()
{
	Token tok = getCurtok();
	if (tok.isSign() && isBracket(tok.getSignType()))
		consumeBracket(tok.getSignType());
	else
	{
		// In all other cases, we can just skip the token, since there's no particular thing to do for other token types.
		skipToken();
	}
}

void Parser::skipToken()
{
	if (state_.tokenIterator != tokens_.end())
		state_.tokenIterator++;
}

void Parser::revertConsume()
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
	Token t = getCurtok();
	if (t.isKeyword())
	{
		skipToken();
		PrimitiveType* ty;
		switch (t.getKeywordType())
		{
			case KeywordType::KW_INT:
				ty = astcontext_.getPrimitiveIntType();
				break;
			case KeywordType::KW_FLOAT:
				ty = astcontext_.getPrimitiveFloatType();
				break;
			case KeywordType::KW_CHAR:
				ty = astcontext_.getPrimitiveCharType();
				break;
			case KeywordType::KW_STRING:
				ty = astcontext_.getPrimitiveStringType();
				break;
			case KeywordType::KW_BOOL:
				ty = astcontext_.getPrimitiveBoolType();
				break;
			default:
				revertConsume();
				return Result<Type*>::NotFound();
		}
		return Result<Type*>(ty,t.sourceRange);
	}
	return Result<Type*>::NotFound();
}

Parser::Result<Type*> Parser::parseType()
{
	// <type> = <builtin_type_name> { '[' ']' }
	// <builtin_type_name> 
	if (auto ty_res = parseBuiltinTypename())
	{
		//  { '[' ']' }
		Type* ty = ty_res.get();
		while (consumeBracket(SignType::S_SQ_OPEN))
		{
			ty = astcontext_.getArrayTypeForType(ty);
			// ']'
			if (!consumeBracket(SignType::S_SQ_CLOSE))
			{
				errorExpected("Expected ']'");

				if (resyncToSign(SignType::S_SQ_CLOSE,/*stopAtSemi */ true ,/*shouldConsumeToken*/ true))
					continue;

				return Result<Type*>::Error();
			}
		}
		return Result<Type*>(ty,ty_res.getSourceRange());
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

bool Parser::resyncToSign(const SignType & sign, const bool & stopAtSemi, const bool & shouldConsumeToken, SourceRange* range)
{
	return resyncToSign(std::vector<SignType>({ sign }), stopAtSemi, shouldConsumeToken, range);
}

bool Parser::resyncToSign(const std::vector<SignType>& signs, const bool & stopAtSemi, const bool & shouldConsumeToken, SourceRange* range)
{
	// Note, this function is heavily based on (read: nearly copy pasted from) CLang's http://clang.llvm.org/doxygen/Parse_2Parser_8cpp_source.html#l00245
	// This is CLang's license https://github.com/llvm-mirror/clang/blob/master/LICENSE.TXT. 
	// As this is not a pure copy-paste but more of a translation & adaptation I don't think I need to link it, but here is it anyways.

	// Return immediately if recovery is not allowed, or the parser isn't alive anymore.
	if (!state_.isRecoveryAllowed || !isAlive())
		return false;

	// Always skip the first token if it's not in signs
	bool isFirst = true;
	// Keep going until we reach EOF.
	while(!isDone())
	{
		// Check curtok
		auto tok = getCurtok();
		for (auto it = signs.begin(); it != signs.end(); it++)
		{
			if (tok.is(*it))
			{
				if (shouldConsumeToken)
					consumeAny();

				if (range)
					*range = tok.sourceRange;

				return true;
			}
		}

		// Check if it's a sign for special behaviours
		if (tok.isSign())
		{
			switch (tok.getSignType())
			{
				// If we find a '(', '{' or '[', call this function recursively to skip to it's counterpart.
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
					// If we find a ')', '}' or ']' we  :
						// Check if it belongs to a unmatched counterpart, if so, stop resync attempt.
						// If it doesn't have an opening counterpart, skip it.
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
					// Intentional fallthrough
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
	if (!state_.isRecoveryAllowed || !isAlive())
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
	if(context_.flagsManager.isSet(FlagID::parser_showRecoveryMessages))
		genericError("Couldn't recover from errors, stopping parsing.");

	state_.tokenIterator = tokens_.end();
	state_.isAlive = false;
}

void Parser::recordDecl(NamedDecl * nameddecl)
{
	// Only assert when we're not in test mode.
	// Tests may call individual parsing function, and won't care about if a DeclRecorder is active or not.

	if (!isTestMode_)
	{
		assert(state_.declRecorder && "Decl Recorder cannot be null when parsing a Declaration!");
	}

	if(state_.declRecorder)
		state_.declRecorder->recordDecl(nameddecl);
}

void Parser::errorUnexpected()
{
	if (!state_.isAlive) return;
	if (isCurrentTokenLastUnexpectedToken()) return;

	markAsLastUnexpectedToken(state_.lastUnexpectedTokenIt);

	context_.setOrigin("Parser");

	std::stringstream output;
	auto tok = getCurtok();
	if (tok)
	{
		output << "Unexpected token \"" << tok.getAsString();
		context_.reportError(output.str());
	}
	context_.resetOrigin();
}

void Parser::errorExpected(const std::string & s)
{
	if (!state_.isAlive) return;

	// Print "unexpected token" error.
	errorUnexpected();

	// set error origin
	context_.setOrigin("Parser");

	std::stringstream output;
	
	if (auto prevtok = getPreviousToken())
		output << s << " after \"" << prevtok.getAsString();
	else
	{
		// We expect a token as first token (?!), print a "before token" error instead of "after" 
		auto tok = getCurtok();
		assert(tok && "Both getPreviousToken() and getCurtok() return invalid tokens?");
		output << s << " before \"" << tok.getAsString();
	}

	context_.reportError(output.str());
	context_.resetOrigin();
}

void Parser::genericError(const std::string & s)
{
	if (!state_.isAlive) return;

	context_.setOrigin("Parser");
	context_.reportError(s);
	context_.resetOrigin();
}

bool Parser::isCurrentTokenLastUnexpectedToken() const
{
	return (state_.tokenIterator == state_.lastUnexpectedTokenIt);
}

void Parser::markAsLastUnexpectedToken(TokenIteratorTy it)
{
	state_.lastUnexpectedTokenIt = it;
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

// RAIIRecoveryManager
Parser::RAIIRecoveryManager::RAIIRecoveryManager(Parser &parser, const bool & allowsRecovery) : parser_(parser)
{
	recoveryAllowedBackup_ = parser_.state_.isRecoveryAllowed;
	parser_.state_.isRecoveryAllowed = allowsRecovery;
}

Parser::RAIIRecoveryManager::~RAIIRecoveryManager()
{
	parser_.state_.isRecoveryAllowed = recoveryAllowedBackup_;
}

Parser::RAIIRecoveryManager Parser::createRecoveryEnabler()
{
	return RAIIRecoveryManager(*this,true);
}

Parser:: RAIIRecoveryManager Parser::createRecoveryDisabler()
{
	return RAIIRecoveryManager(*this, false);
}

// RAIIDeclRecorder
Parser::RAIIDeclRecorder::RAIIDeclRecorder(Parser &p, DeclRecorder *dr) : parser(p)
{
	old_dc = parser.state_.declRecorder;

	// If old_dc isn't null, mark it as the parent of the new dr
	if (old_dc)
	{
		// Assert that we're not overwriting a parent. If such a thing happens, that could indicate a bug!
		assert(!dr->hasParentDeclRecorder() && "New DeclRecorder already has a parent?");
		dr->setParentDeclRecorder(old_dc);
	}

	parser.state_.declRecorder = dr;
}

Parser::RAIIDeclRecorder::~RAIIDeclRecorder()
{
	parser.state_.declRecorder = old_dc;
}