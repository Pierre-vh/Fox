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

// Stringstream
#include <sstream>
// Context and Exceptions
#include "Moonshot/Common/Context/Context.hpp"
#include "Moonshot/Common/Exceptions/Exceptions.hpp"

using namespace Moonshot;

#define RETURN_IF_DEAD 	if (!state_.isAlive) return

Parser::Parser(Context& c, TokenVector& l) : context_(c),tokens_(l)
{

}

ParsingResult<ASTUnit*> Parser::parseUnit()
{
	// <fox_unit>	= {<declaration>}1+
	// Note: unit "hard fails" if it did not find any declaration, since it's a mandatory rule for any file.
	std::size_t counter = 0;
	auto unit = std::make_unique<ASTUnit>();
	// Parse declarations 
	while (true)
	{
		// Parse a declaration
		auto decl = parseTopLevelDecl();
		// If the declaration was parsed successfully : continue the cycle.
		if (decl)
		{
			counter++;
			unit->addDecl(std::move(decl.result_));
			continue;
		}
		else
		{
			// Act differently depending on the failure type
			if (decl.getFlag() == ParsingOutcome::NOTFOUND)
			{
				// if it didn't find it because we reached EOF, that means our job is done
				if (hasReachedEndOfTokenStream())
					break;
				else // It didn't find a declaration and no eof
				{
					// Report an error
					errorUnexpected();
					genericError("Attempting recovery to next declaration...");
					// Try to go to the next decl
					if (resyncToNextDeclKeyword())
					{
						genericError("Recovered successfully."); // Note : add a position, like "Recovered successfuly at line x"
						continue;
					}
					else	// Died, break.
						break;
				}
			}
			// Other error, and there's still stuff to parse, try to recover.
			else if (!hasReachedEndOfTokenStream())
			{
				// If the decl has error, but still managed to return something, make use of it!
				if (decl.isDataAvailable())
				{
					counter++;
					unit->addDecl(std::move(decl.result_));
				}
				genericError("Attempting recovery to next declaration.");
				if (resyncToNextDeclKeyword())
				{
					genericError("Recovered successfully."); // Note : add a position, like "Recovered successfuly at line x"
					continue;
				}
				else	// Died, break.
					break;
			}
			else // Other error and reached  eof, break.
				break;
		}

	}
	if (counter == 0)
		genericError("Expected one or more declaration in unit.");

	// Return !
		// Note:  we always return the Unit, because even if it's incomplete, we might want to dump it to see what was parsed successfuly.
	if (isAlive() && counter) // If parser is alive and we got 1 or more decl
		return ParsingResult<ASTUnit*>(ParsingOutcome::SUCCESS, std::move(unit));
	else if (!isAlive()) // Died :(
		return ParsingResult<ASTUnit*>(ParsingOutcome::FAILED_AND_DIED, std::move(unit));
	else if (counter && hasReachedEndOfTokenStream()) // Parser is alive, we found 1 or more decl and we reached end of stream... we had errors but recovered successfuly.
		return ParsingResult<ASTUnit*>(ParsingOutcome::FAILED_BUT_RECOVERED, std::move(unit));
	else 
		return ParsingResult<ASTUnit*>(ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY, std::move(unit));
}

ParsingResult<FoxValue> Parser::matchLiteral()
{
	Token t = getToken();
	if (t.isLiteral())
	{
		if (auto litinfo = t.getLiteralInfo())
		{
			state_.pos += 1;

			FoxValue fval;
			// Convert to fval (temporary solution until ast & type rework)
			if (litinfo.is<bool>())
				fval = litinfo.get<bool>();
			else if (litinfo.is<std::string>())
				fval = litinfo.get<std::string>();
			else if (litinfo.is<float>())
				fval = litinfo.get<float>();
			else if (litinfo.is<int64_t>())
				fval = litinfo.get<int64_t>();
			else if (litinfo.is<char32_t>())
				fval = litinfo.get<char32_t>();

			return ParsingResult<FoxValue>(ParsingOutcome::SUCCESS, fval);
		}
		else
			throw std::exception("Returned an invalid litinfo when the token was a literal?");
	}
	return ParsingResult<FoxValue>(ParsingOutcome::NOTFOUND);
}

ParsingResult<std::string> Parser::matchID()
{
	Token t = getToken();
	if (t.isIdentifier())
	{
		incrementPosition();
		return ParsingResult<std::string>(ParsingOutcome::SUCCESS, t.getString());
	}
	return ParsingResult<std::string>(ParsingOutcome::NOTFOUND);
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

ParsingResult<std::size_t> Parser::matchTypeKw()
{
	Token t = getToken();
	incrementPosition();
	if (t.isKeyword())
	{
		switch (t.getKeywordType())
		{
			case KeywordType::KW_INT:	return  ParsingResult<std::size_t>(ParsingOutcome::SUCCESS, TypeIndex::basic_Int);
			case KeywordType::KW_FLOAT:	return  ParsingResult<std::size_t>(ParsingOutcome::SUCCESS, TypeIndex::basic_Float);
			case KeywordType::KW_CHAR:	return  ParsingResult<std::size_t>(ParsingOutcome::SUCCESS, TypeIndex::basic_Char);
			case KeywordType::KW_STRING:	return  ParsingResult<std::size_t>(ParsingOutcome::SUCCESS, TypeIndex::basic_String);
			case KeywordType::KW_BOOL:	return  ParsingResult<std::size_t>(ParsingOutcome::SUCCESS,TypeIndex::basic_Bool);
		}
	}
	decrementPosition();
	return ParsingResult<std::size_t>(ParsingOutcome::NOTFOUND);
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

std::size_t Parser::getNextPosition() const
{
	return state_.pos + 1;
}

void Parser::incrementPosition()
{
	state_.pos+=1;
}

void Parser::decrementPosition()
{
	state_.pos-=1;
}

bool Parser::resyncToSign(const SignType & s)
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
					return true;
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
		if (tok.getString().size() == 1)
			output << "Unexpected char '" << tok.getString() << "' at line " << tok.getPosition().line;
		else
			output << "Unexpected Token \xAF" << tok.getString() << "\xAE at line " << tok.getPosition().line;
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
	output << s << " after \"" << tok.getString() << "\" at line " << tok.getPosition().line;

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

