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

Parser::Parser(Context& c, Lexer& l) : context_(c),lex_(l)
{

}

Parser::~Parser()
{
}

std::pair<bool, token> Parser::matchValue()
{
	token t = getToken();
	if (t.type == tokenType::TT_VALUE)
	{
		pos_ += 1;
		return { true,t };
	}
	return { false,token(Context()) };
}



std::pair<bool, std::string> Parser::matchID()
{
	token t = getToken();
	if (t.type == tokenType::TT_IDENTIFIER)
	{
		pos_ += 1;
		return { true, t.str };
	}
	return { false, "" };
}

bool Parser::matchSign(const signType & s)
{
	token t = getToken();
	if (t.type == tokenType::TT_SIGN && t.sign_type == s)
	{
		pos_ += 1;
		return true;
	}
	return false;
}

bool Parser::matchKeyword(const keywordType & k)
{
	token t = getToken();
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
	token t = getToken();
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

token Parser::getToken() const
{
	return getToken(pos_);
}

token Parser::getToken(const size_t & d) const
{
	if (d < lex_.resultSize())
		return lex_.getToken(d);
	else
		return token(Context());
}

void Parser::errorUnexpected()
{
	context_.setOrigin("Parser");

	std::stringstream output;
	output << "Unexpected token " << getToken().showFormattedTokenData() << std::endl;
	context_.reportError(output.str());

	context_.resetOrigin();
}

void Parser::errorExpected(const std::string & s)
{
	context_.setOrigin("Parser");

	std::stringstream output;
	output << s << "\n[after token " << getToken(pos_-1).showFormattedTokenData() << "]" << std::endl;
	context_.reportError(output.str());

	context_.resetOrigin();
}
