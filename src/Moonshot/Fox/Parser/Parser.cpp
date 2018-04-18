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

#define RETURN_IF_DEAD 	if (!state_.isAlive) return

Parser::Parser(Context& c, ASTContext& astctxt, TokenVector& l) : context_(c), astcontext_(astctxt), tokens_(l)
{

}

UnitParsingResult Parser::parseUnit()
{
	// <fox_unit>	= {<declaration>}1+
	auto unit = std::make_unique<ASTUnit>();
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
					errorUnexpected();
					genericError("Attempting recovery to next declaration.");
					if (resyncToNextDeclKeyword())
					{
						genericError("Recovered successfully."); // Note : add a position, like "Recovered successfuly at line x"
						continue;
					}
					else // Break so we can return
						break;
				}
			}
			// Parsing failed ? Try to recover!
			else 
			{
				genericError("Attempting recovery to next declaration.");
				if (resyncToNextDeclKeyword())
				{
					genericError("Recovered successfully."); // Note : add a position, like "Recovered successfuly at line x"
					continue;
				}
				else // Break so we can return.
					break;
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
		state_.pos += 1;
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

std::pair<const Type*, bool> Parser::parseType(const bool& recoveryAllowed)
{
	// <type> = <builtin_type_name> { '[' ']' }
	// <builtin_type_name> 
	if (auto ty = parseBuiltinTypename())
	{
		//  { '[' ']' }
		while (matchSign(SignType::S_SQ_OPEN))
		{
			// ']'
			if (matchSign(SignType::S_SQ_CLOSE))
			{
				// Found ']'
				// Nest the array one more level
				ty = astcontext_.getArrayTypeForType(ty);
			}
			else
			{
				errorExpected("Expected ']'");
				// not found, try resync if allowed
				if (recoveryAllowed)
				{
					// if resync, good
					if (resyncToSign(SignType::S_SQ_CLOSE))
						ty = astcontext_.getArrayTypeForType(ty);
					else // if can't resync, error.
						return { nullptr , false };
				}
				else // if not allowed to resync, return error now.
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
	return getToken(state_.pos);
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
	return state_.pos;
}

void Parser::incrementPosition()
{
	state_.pos+=1;
}

void Parser::decrementPosition()
{
	state_.pos-=1;
}

bool Parser::resyncToSign(const SignType & s, const bool& consumeToken)
{
	if (isClosingDelimiter(s))
	{
		std::size_t counter = 0;
		auto opener = getOppositeDelimiter(s);
		for (; state_.pos < tokens_.size(); state_.pos++)
		{
			if (matchSign(opener))
			{
				counter++;
				continue;
			}
			if (matchSign(s))
			{
				if (counter)
					counter--;
				else
				{
					if(consumeToken)
						return true;
					else
					{
						// if the token shouldn't be consumed, go back 1 token and return, so the token
						// is left to be picked up by another parsing function
						decrementPosition();
						return true;
					}
				}
			}
		}
	}
	else
	{
		for (; state_.pos < tokens_.size(); state_.pos++)
		{
			if (matchSign(s))
				return true;
		}
	}
	die();
	return false;
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

bool Parser::resyncToNextDeclKeyword()
{
	for (; state_.pos < tokens_.size(); state_.pos++)
	{
		if (matchKeyword(KeywordType::KW_FUNC) || matchKeyword(KeywordType::KW_LET))
		{
			// Decrement, so we reverse the token consuming and make the let/func available to be picked up by parseDecl
			decrementPosition();
			return true;
		}
	}
	die();
	return false;
}

void Parser::die()
{
	genericError("Couldn't recover from error, stopping parsing.");

	state_.pos = tokens_.size();
	state_.isAlive = false;
}

void Parser::errorUnexpected()
{
	RETURN_IF_DEAD;

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

	RETURN_IF_DEAD;
	const auto lastTokenPos = state_.pos - 1;

	// If needed, print unexpected error message
	if (lastUnexpectedTokenPosition != state_.pos)
	{
		lastUnexpectedTokenPosition = state_.pos;
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
	RETURN_IF_DEAD;

	context_.setOrigin("Parser");
	context_.reportError(s);
	context_.resetOrigin();
}

bool Parser::hasReachedEndOfTokenStream() const
{
	return (state_.pos >= tokens_.size());
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

