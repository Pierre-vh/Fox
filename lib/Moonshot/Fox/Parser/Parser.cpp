////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Parser.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
//	This file implements rule-agnostic methods				
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

Parser::~Parser()
{
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
		state_.pos += 1;
		return ParsingResult<std::string>(ParsingOutcome::SUCCESS, t.getString());
	}
	return ParsingResult<std::string>(ParsingOutcome::NOTFOUND);
}

bool Parser::matchSign(const SignType & s)
{
	Token t = getToken();
	if (t.isSign() && (t.getSignType() == s))
	{
		state_.pos += 1;
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
	state_.pos += 1;
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
	state_.pos -= 1;
	return ParsingResult<std::size_t>(ParsingOutcome::NOTFOUND);
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

bool Parser::resyncToDelimiter(const SignType & s)
{
	for (; state_.pos < tokens_.size(); state_.pos++)
	{
		auto tok = getToken();
		if (tok.isSign() && (tok.getSignType() == s))
		{
			state_.pos++;
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

void Parser::errorExpected(const std::string & s, const std::vector<std::string>& sugg)
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
	output << s << " after \xAF" << tok.getString() << "\xAE at line " << tok.getPosition().line;

	if (sugg.size())
	{
		output << "\n\tSuggestions:\n";
		for (auto& elem : sugg)
		{
			output << "\t\t" << elem << "\n";
		}
	}

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

Parser::ParserState Parser::createParserStateBackup() const
{
	return state_;
}

void Parser::restoreParserStateFromBackup(const Parser::ParserState & st)
{
	state_ = st;
}
