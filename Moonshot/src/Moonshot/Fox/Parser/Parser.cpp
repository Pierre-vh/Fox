
/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : This class implements the Parser Class and the general parsing rules

*************************************************************
MIT License

Copyright (c) 2017 Pierre van Houtryve

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*************************************************************/

#include "Parser.h"

using namespace Moonshot;

Parser::Parser(Lexer *l) : lex_(l)
{
}

Parser::~Parser()
{
}

std::unique_ptr<ASTExpr> Parser::oneUpNode(std::unique_ptr<ASTExpr>& node, const parse::optype & op)
{
	auto newnode = std::make_unique<ASTExpr>(op);
	newnode->makeChild(parse::LEFT, node);
	return newnode;
}

bool Parser::matchValue(const lex::values & v)
{
	token t = getToken();
	if(t.type == lex::TT_VALUE && t.val_type == v)
	{
		pos_ += 1;
		return true;
	}
	return false;
}

bool Parser::matchID()
{
	token t = getToken();
	if (t.type == lex::TT_IDENTIFIER)
	{
		pos_ += 1;
		return true;
	}
	return false;
}

bool Parser::matchSign(const lex::signs & s)
{
	token t = getToken();
	if (t.type == lex::TT_SIGN && t.sign_type == s)
	{
		pos_ += 1;
		return true;
	}
	return false;
}

bool Parser::matchKeyword(const lex::keywords & k)
{
	token t = getToken();
	if (t.type == lex::TT_KEYWORD && t.kw_type == k)
	{
		pos_ += 1;
		return true;
	}
	return false;
}

std::pair<bool, parse::types> Moonshot::Parser::matchTypeKw()
{
	token t = getToken();
	if (t.type == lex::TT_KEYWORD)
	{
		parse::types rtr = parse::NOCAST;
		switch (t.kw_type)
		{
			case lex::T_BOOL:
				rtr = parse::TYPE_BOOL;
				break;
			case lex::T_INT:
				rtr = parse::TYPE_INT;
				break;
			case lex::T_FLOAT:
				rtr = parse::TYPE_FLOAT;
				break;
			case lex::T_CHAR:
				rtr = parse::TYPE_CHAR;
				break;
			case lex::T_STRING:
				rtr = parse::TYPE_STR;
				break;
		}
		if (rtr != parse::NOCAST)
		{
			pos_ += 1;
			return { true,rtr };
		}
	}
	return { false, parse::NOCAST };
}

token Parser::getToken() const
{
	return getToken(pos_);
}

token Parser::getToken(const size_t & d) const
{
	if (d < lex_->resultSize())
		return lex_->getToken(d);
	else
		return token();
}

void Moonshot::Parser::errorUnexpected()
{
	std::stringstream ss;
	ss << " Unexpected token " << getToken().showFormattedTokenData() << std::endl;
	E_ERROR(ss.str());
}

void Moonshot::Parser::errorExpected(const std::string & s)
{
	std::stringstream ss;
	ss << s << " after token " << getToken().showFormattedTokenData() << std::endl;
	E_ERROR(ss.str());
}
