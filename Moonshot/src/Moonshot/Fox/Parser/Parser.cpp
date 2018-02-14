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
using namespace fv_util;

#define RETURN_IF_DEAD 	if (!state_.isAlive) return

Parser::Parser(Context& c, TokenVector& l) : context_(c),tokens_(l)
{
	maxExpectedErrorCount_ = context_.optionsManager_.getAttr(OptionsList::parser_maxExpectedErrorCount).value_or(DEFAULT__maxExpectedErrorsCount).get<int>();
	shouldPrintSuggestions_ = context_.optionsManager_.getAttr(OptionsList::parser_printSuggestions).value_or(DEFAULT__shouldPrintSuggestions).get<bool>();
}

Parser::~Parser()
{
}

std::pair<bool, Token> Parser::matchLiteral()
{
	Token t = getToken();
	if (t.type == tokenCat::LITERAL)
	{
		state_.pos += 1;
		return { true,t };
	}
	return { false,Token(Context()) };
}



std::pair<bool, std::string> Parser::matchID()
{
	Token t = getToken();
	if (t.type == tokenCat::IDENTIFIER)
	{
		state_.pos += 1;
		return { true, t.str };
	}
	return { false, "" };
}

bool Parser::matchSign(const sign & s)
{
	Token t = getToken();
	if (t.type == tokenCat::SIGN && t.sign_type == s)
	{
		state_.pos += 1;
		return true;
	}
	return false;
}

bool Parser::matchKeyword(const keyword & k)
{
	Token t = getToken();
	if (t.type == tokenCat::KEYWORD && t.kw_type == k)
	{
		state_.pos += 1;
		return true;
	}
	return false;
}


std::size_t Parser::matchTypeKw()
{
	Token t = getToken();
	state_.pos += 1;
	if (t.type == tokenCat::KEYWORD)
	{
		switch (t.kw_type)
		{
			case keyword::T_INT:	return indexes::fval_int;
			case keyword::T_FLOAT:	return indexes::fval_float;
			case keyword::T_CHAR:	return indexes::fval_char;
			case keyword::T_STRING:	return indexes::fval_str;
			case keyword::T_BOOL:	return indexes::fval_bool;
		}
	}
	state_.pos -= 1;
	return indexes::invalid_index;
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
		return Token(Context());
}


bool Parser::resyncToDelimiter(const sign & s)
{
	for (; state_.pos < tokens_.size(); state_.pos++)
	{
		if (getToken().sign_type == s)
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
	if (tok.str.size())
	{
		if(tok.str.size() == 1)
			output << "Unexpected char '" << tok.str << "' at line " << tok.pos.line;
		else
			output << "Unexpected Token \xAF" << tok.str << "\xAE at line " << tok.pos.line;
		context_.reportError(output.str());
	}
	context_.resetOrigin();
}

void Parser::errorExpected(const std::string & s, const std::vector<std::string>& sugg)
{
	RETURN_IF_DEAD;

	static std::size_t lastUnexpectedTokenPosition;
	if (currentExpectedErrorsCount_ > maxExpectedErrorCount_)
		return;

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
	if(tok.str.size()==1)
		output << s << " after '" << tok.str << "' at line " << tok.pos.line;
	else 
		output << s << " after \xAF" << tok.str << "\xAE at line " << tok.pos.line;

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
	currentExpectedErrorsCount_++;
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
