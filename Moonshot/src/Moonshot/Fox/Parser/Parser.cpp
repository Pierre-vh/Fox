////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Parser.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
//	This file implements rule-agnostic methods				
////------------------------------------------------------////

#include "Parser.h"

using namespace Moonshot;
using namespace fv_util;

Parser::Parser(Context& c, TokenVector& l) : context_(c),tokens_(l)
{

}

Parser::~Parser()
{
}

std::pair<bool, Token> Parser::matchLiteral()
{
	Token t = getToken();
	if (t.type == tokenType::TT_LITERAL)
	{
		pos_ += 1;
		return { true,t };
	}
	return { false,Token(Context()) };
}



std::pair<bool, std::string> Parser::matchID()
{
	Token t = getToken();
	if (t.type == tokenType::TT_IDENTIFIER)
	{
		pos_ += 1;
		return { true, t.str };
	}
	return { false, "" };
}

bool Parser::matchSign(const signType & s)
{
	Token t = getToken();
	if (t.type == tokenType::TT_SIGN && t.sign_type == s)
	{
		pos_ += 1;
		return true;
	}
	return false;
}

bool Parser::matchKeyword(const keywordType & k)
{
	Token t = getToken();
	if (t.type == tokenType::TT_KEYWORD && t.kw_type == k)
	{
		pos_ += 1;
		return true;
	}
	return false;
}

bool Parser::matchEOI()
{
	return matchSign(signType::P_SEMICOLON);
}

std::size_t Moonshot::Parser::matchTypeKw()
{
	Token t = getToken();
	pos_ += 1;
	if (t.type == tokenType::TT_KEYWORD)
	{
		switch (t.kw_type)
		{
			case keywordType::T_INT:	return fval_int;
			case keywordType::T_FLOAT:	return fval_float;
			case keywordType::T_CHAR:	return fval_char;
			case keywordType::T_STRING:	return fval_str;
			case keywordType::T_BOOL:	return fval_bool;
		}
	}
	pos_ -= 1;
	return invalid_index;
}

Token Parser::getToken() const
{
	return getToken(pos_);
}

Token Parser::getToken(const size_t & d) const
{
	if (d < tokens_.size())
		return tokens_.at(d);
	else
		return Token(Context());
}

void Parser::errorUnexpected()
{
	context_.setOrigin("Parser");

	std::stringstream output;
	output << "Unexpected Token " << getToken().showFormattedTokenData();
	context_.reportError(output.str());

	context_.resetOrigin();
}

void Parser::errorExpected(const std::string & s)
{
	static std::size_t lastErrorPosition;
	if (lastErrorPosition != pos_)
		errorUnexpected();
	lastErrorPosition = pos_;

	context_.setOrigin("Parser");

	std::stringstream output;
	output << s << "\n[after Token " << getToken(pos_-1).showFormattedTokenData() << "]";
	context_.reportError(output.str());

	context_.resetOrigin();
}
