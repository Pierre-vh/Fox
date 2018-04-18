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
#include "Moonshot/Fox/AST/IdentifierTable.hpp"
#include "Moonshot/Fox/Basic/Context.hpp"
#include "Moonshot/Fox/Basic/Exceptions.hpp"

using namespace Moonshot;

Parser::Parser(Context& c, ASTContext& astctxt, TokenVector& l) : context_(c), astcontext_(astctxt), tokens_(l)
{

}

UnitParsingResult Parser::parseUnit()
{
	// <fox_unit>	= {<declaration>}1+
	auto unit = std::make_unique<ASTUnit>();

	// Create recovery "lock" object, since recovery is disabled for top level declarations.
	auto lock = createRecoveryDisabler();

	// Gather some flags
	const bool showRecoveryMessages = context_.flagsManager.isSet(FlagID::parser_showRecoveryMessages);

	// Parse declarations 
	while (true)
	{
		// Parse a declaration
		auto decl = parseDecl();
		// If the declaration was parsed successfully : continue the cycle.
		if (decl)
		{
			std::cout << "pushed decl\n";
			unit->addDecl(std::move(decl.result));
			continue;
		}
		else
		{
			// If the ParsingResult is not usable, but the parsing was successful
			if (decl.wasSuccessful()) 
			{
				// Parsing was successful & EOF -> Job's done !
				if (hasReachedEndOfTokenStream())
					break;
				// No EOF? There's an unexpected token on the way, report it & recover!
				else
				{
					// Unlock, so we're allowed to recover here.
					auto unlock = createRecoveryEnabler();

					errorUnexpected();

					if (showRecoveryMessages)
						genericError("Attempting recovery to next declaration.");

					if (resyncToNextDeclKeyword())
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
			// Parsing failed ? Try to recover!
			else 
			{
				// Unlock, so we're allowed to recover here.
				auto unlock = createRecoveryEnabler();

				if(showRecoveryMessages)
					genericError("Attempting recovery to next declaration.");

				if (resyncToNextDeclKeyword())
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

	// Return nothing if no declaration was found to report a failure.
	if (unit->getDeclCount() == 0)
	{
		// Unit reports an error if notfound, because it's a mandatory rule.
		genericError("Expected one or more declaration in unit.");
		// Return empty result
		return UnitParsingResult();
	}
	// Return the unit if it's valid.
	else
		return UnitParsingResult(std::move(unit));
}

ParsingResult<LiteralInfo> Parser::matchLiteral()
{
	Token t = getToken();
	if (t.isLiteral())
	{
		incrementPosition();
		if (auto litinfo = t.getLiteralInfo())
			return ParsingResult<LiteralInfo>(litinfo);
		else
			throw std::exception("Returned an invalid litinfo when the token was a literal?");
	}
	return ParsingResult<LiteralInfo>();
}

IdentifierInfo* Parser::matchID()
{
	Token t = getToken();
	if (t.isIdentifier())
	{
		incrementPosition();

		IdentifierInfo* ptr = t.getIdentifierInfo();;
		assert(ptr && "Token's an identifier but contains a nullptr IdentifierInfo?");
		return ptr;
	}
	return nullptr;
}

bool Parser::matchSign(const SignType & s)
{
	if (peekSign(getCurrentPosition(),s))
	{
		incrementPosition();
		return true;
	}
	return false;
}

bool Parser::matchKeyword(const KeywordType & k)
{
	Token t = getToken();
	if (t.isKeyword() && (t.getKeywordType() == k))
	{
		parserState_.pos += 1;
		return true;
	}
	return false;
}

const Type* Parser::parseBuiltinTypename()
{
	// <builtin_type_name> 	= "int" | "float" | "bool" | "string" | "char"
	Token t = getToken();
	incrementPosition();
	if (t.isKeyword())
	{
		switch (t.getKeywordType())
		{
			case KeywordType::KW_INT:	return  astcontext_.getPrimitiveIntType();
			case KeywordType::KW_FLOAT:	return  astcontext_.getPrimitiveFloatType();
			case KeywordType::KW_CHAR:	return	astcontext_.getPrimitiveCharType();
			case KeywordType::KW_STRING:return	astcontext_.getPrimitiveStringType();
			case KeywordType::KW_BOOL:	return	astcontext_.getPrimitiveBoolType();
		}
	}
	decrementPosition();
	return nullptr;
}

std::pair<const Type*, bool> Parser::parseType()
{
	// <type> = <builtin_type_name> { '[' ']' }
	// <builtin_type_name> 
	if (auto ty = parseBuiltinTypename())
	{
		//  { '[' ']' }
		while (matchSign(SignType::S_SQ_OPEN))
		{
			// Set ty to the ArrayType of ty.
			ty = astcontext_.getArrayTypeForType(ty);
			// ']'
			if (!matchSign(SignType::S_SQ_CLOSE))
			{
				errorExpected("Expected ']'");
				// if we can't recover, report an error.
				if (auto rres = resyncToSignInStatement(SignType::S_SQ_CLOSE))
				{
					// if recovered on requested token, continue..
					if (rres.hasRecoveredOnRequestedToken())
						continue;
				}
				return { nullptr , false };
			}
		}
		// found, return
		return { ty, true };
	}
	// notfound, return
	return { nullptr, true };
}

bool Parser::peekSign(const std::size_t & idx, const SignType & sign) const
{
	if (auto tok = getToken(idx))
		return tok.isSign() && (tok.getSignType() == sign);
	return false;
}

Token Parser::getToken() const
{
	return getToken(parserState_.pos);
}

Token Parser::getToken(const size_t & d) const
{
	if (d < tokens_.size())
		return tokens_.at(d);
	else
		return Token();
}

std::size_t Parser::getCurrentPosition() const
{
	return parserState_.pos;
}

void Parser::incrementPosition()
{
	parserState_.pos+=1;
}

void Parser::decrementPosition()
{
	parserState_.pos-=1;
}

//Skips every token until the sign s,a semicolon, "func", eof or a token marking the beginning of a statement is found.
ResyncResult Parser::resyncToSignInStatement(const SignType & s, const bool & consumeToken)
{
	// Abort if recovery is forbidden
	if (!parserState_.isRecoveryAllowed)
		return ResyncResult(false); 

	std::size_t counter = 0;
	auto opener = getOppositeDelimiter(s);
	bool hasOpener = isClosingDelimiter(s);
	bool isRequestingSemi = (s == SignType::S_SEMICOLON);
	for (; parserState_.pos < tokens_.size(); parserState_.pos++)
	{
		auto tok = getToken();
		if (tok.isKeyword())
		{
			auto kwTy = tok.getKeywordType();
			if (isBeginningOfStatementKeyword(kwTy) || (kwTy == KeywordType::KW_FUNC))
			{
				// if the user requests a semicolon, we return a success, because we found another "statement delimiter"
				// if he wasn't requesting one, that's just a match failure.
				return ResyncResult(isRequestingSemi, false);
			}
		}
		else if(tok.isSign())
		{
			auto signTy = tok.getSignType();
			if (signTy == SignType::S_SEMICOLON)
			{
				if (isRequestingSemi && consumeToken) 
					incrementPosition();
				// if the user is requesting a semicolon and we match one, it's a success (true,true)
				// if the user isn't requesting to match a semi, that's considered a failure (false,false)
				return ResyncResult(isRequestingSemi, isRequestingSemi);
			}
			else if (signTy == s)
			{
				if (hasOpener && counter)
					counter--;
				else
				{
					if (consumeToken) 
						incrementPosition();
					return ResyncResult(true, true);
				}
			}	
			else if (hasOpener && (signTy == opener))
				counter++;
		}
	}
	die();
	return ResyncResult(false);
}

//Skips every token until the sign s, "func" or eof is found.
ResyncResult Parser::resyncToSignInFunction(const SignType & s, const bool & consumeToken)
{
	// Abort if recovery is forbidden
	if (!parserState_.isRecoveryAllowed)
		return false;

	std::size_t counter = 0;
	auto opener = getOppositeDelimiter(s);
	bool hasOpener = isClosingDelimiter(s);

	for (; parserState_.pos < tokens_.size(); parserState_.pos++)
	{
		auto tok = getToken();
		if (tok.isKeyword())
		{
			auto kwTy = tok.getKeywordType();
			if (kwTy == KeywordType::KW_FUNC)
			{
				// Return false if a func is found, because that means that we couldn't recover.
				return ResyncResult(false);
			}
		}
		else if (tok.isSign())
		{
			auto signTy = tok.getSignType();
			if (hasOpener && (signTy == opener))
				counter++;
			else if (signTy == s)
			{
				if (hasOpener && counter)
					counter--;
				else
				{
					if (consumeToken) 
						incrementPosition();
					return ResyncResult(true,true);
				}
			}
		}
	}
	die();
	return ResyncResult(false);
}

ResyncResult Parser::resyncToNextDeclKeyword()
{
	// Check if recovery is allowed 
	if (!parserState_.isRecoveryAllowed)
		return ResyncResult(false);

	for (; parserState_.pos < tokens_.size(); parserState_.pos++)
	{
		if (matchKeyword(KeywordType::KW_FUNC) || matchKeyword(KeywordType::KW_LET))
		{
			// Decrement, so we reverse the token consuming and make the let/func available to be picked up by parseDecl
			decrementPosition();
			return ResyncResult(true,true);
		}
	}
	die();
	return ResyncResult(false);
}

bool Parser::isBeginningOfStatementKeyword(const KeywordType & kw)
{
	// Returns true if kw is on of "let", "if", "else", "while", "return" or "func"
	switch (kw)
	{
		case KeywordType::KW_LET:
		case KeywordType::KW_IF:
		case KeywordType::KW_ELSE:
		case KeywordType::KW_WHILE:
		case KeywordType::KW_RETURN:
		case KeywordType::KW_FUNC:
			return true;
		default:
			return false;
	}
}

bool Parser::isClosingDelimiter(const SignType & s) const
{
	return (s == SignType::S_CURLY_CLOSE) || (s == SignType::S_ROUND_CLOSE) || (s == SignType::S_SQ_CLOSE);
}

SignType Parser::getOppositeDelimiter(const SignType & s)
{
	if (s == SignType::S_CURLY_CLOSE)
		return SignType::S_CURLY_OPEN;

	if (s == SignType::S_ROUND_CLOSE)
		return SignType::S_ROUND_OPEN;

	if (s == SignType::S_SQ_CLOSE)
		return SignType::S_SQ_OPEN;

	return SignType::DEFAULT;
}

void Parser::die()
{
	if(context_.flagsManager.isSet(FlagID::parser_showRecoveryMessages))
		genericError("Couldn't recover from error, stopping parsing.");

	parserState_.pos = tokens_.size();
	parserState_.isAlive = false;
}

void Parser::errorUnexpected()
{
	if (!parserState_.isAlive) return;

	context_.setOrigin("Parser");

	std::stringstream output;
	auto tok = getToken();
	if (tok)
	{
		if (tok.getAsString().size() == 1)
			output << "Unexpected char '" << tok.getAsString() << "' at line " << tok.getPosition().line;
		else
			output << "Unexpected Token \xAF" << tok.getAsString() << "\xAE at line " << tok.getPosition().line;
		context_.reportError(output.str());
	}
	context_.resetOrigin();
}

void Parser::errorExpected(const std::string & s)
{
	static std::size_t lastUnexpectedTokenPosition;

	if (!parserState_.isAlive) return;

	const auto lastTokenPos = parserState_.pos - 1;

	// If needed, print unexpected error message
	if (lastUnexpectedTokenPosition != parserState_.pos)
	{
		lastUnexpectedTokenPosition = parserState_.pos;
		errorUnexpected();
	}

	context_.setOrigin("Parser");

	std::stringstream output;
	auto tok = getToken(lastTokenPos);
	output << s << " after \"" << tok.getAsString() << "\" at line " << tok.getPosition().line;

	context_.reportError(output.str());
	context_.resetOrigin();
}

void Parser::genericError(const std::string & s)
{
	if (!parserState_.isAlive) return;

	context_.setOrigin("Parser");
	context_.reportError(s);
	context_.resetOrigin();
}

bool Parser::hasReachedEndOfTokenStream() const
{
	return (parserState_.pos >= tokens_.size());
}

bool Parser::isAlive() const
{
	return parserState_.isAlive;
}

Parser::ParserState Parser::createParserStateBackup() const
{
	return parserState_;
}

void Parser::restoreParserStateFromBackup(const Parser::ParserState & st)
{
	parserState_ = st;
}

// ParserState
Parser::ParserState::ParserState() : isAlive(true), isRecoveryAllowed(false), pos(0)
{

}

// RAIIRecoveryManager

Parser::RAIIRecoveryManager::RAIIRecoveryManager(Parser * parser, const bool & allowsRecovery) : parser_(parser)
{
	assert(parser_ && "Parser instance pointer cannot be null!");
	recoveryAllowedBackup_ = parser_->parserState_.isRecoveryAllowed;
	parser_->parserState_.isRecoveryAllowed = allowsRecovery;
}

Parser::RAIIRecoveryManager::~RAIIRecoveryManager()
{
	assert(parser_ && "Parser instance pointer cannot be null!");
	parser_->parserState_.isRecoveryAllowed = recoveryAllowedBackup_;
}

Parser::RAIIRecoveryManager Parser::createRecoveryEnabler()
{
	return RAIIRecoveryManager(this,true);
}

Parser:: RAIIRecoveryManager Parser::createRecoveryDisabler()
{
	return RAIIRecoveryManager(this, false);
}