
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

std::pair<bool, parse::optype> Parser::matchSecondOp()
{
	// 2nd priority operators: + - || &&
	token t = getToken();
	token pk = getToken(pos_ + 1);
	pos_ += 1;
	// 
	if (t.isValid() && (t.type == lex::TT_SIGN))
	{
		if (t.sign_type == lex::S_PLUS)
			return { true,parse::ADD };
		else if (t.sign_type == lex::S_MINUS)
			return { true,parse::MINUS };
		else if (t.sign_type == lex::S_VBAR)
		{
			if (pk.isValid() && (pk.sign_type == lex::S_VBAR))
			{
				pos_ += 1;
				return { true,parse::OR };
			}
		}
		else if (t.sign_type == lex::S_AND)
		{
			if (pk.isValid() && (pk.sign_type == lex::S_AND))
			{
				pos_ += 1;
				return { true,parse::AND };
			}
		}
	}
	E_LOG("Can't match second op");
	std::cout << "c = " << t.str << ",& pk = " << pk.str << std::endl;
	pos_ -= 1;	// Found nothing : decrement and return.
	return { false, parse::optype::DEFAULT };
}

std::pair<bool, parse::optype> Parser::matchPriorOp()
{
	token t = getToken();
	token pk = getToken(pos_ + 1);
	pos_ += 1;
	// 
	if (t.isValid() && (t.type == lex::TT_SIGN))
	{
		if (t.sign_type == lex::S_ASTERISK)
			return { true,parse::MUL };					// *
		else if (t.sign_type == lex::S_SLASH)
			return { true,parse::DIV };					// /
		else if (t.sign_type == lex::S_LESS_THAN)	
		{
			if (pk.isValid() && (pk.sign_type == lex::S_EQUAL))
			{
				pos_ += 1;
				return { true,parse::LESS_OR_EQUAL };	// <=
			}
			else
				return { true,parse::LESS_THAN };		// <
		}
		else if (t.sign_type == lex::S_GREATER_THAN)
		{
			if (pk.isValid() && (pk.sign_type == lex::S_EQUAL))
			{
				pos_ += 1;
				return { true,parse::GREATER_OR_EQUAL };	// >=
			}
			else
				return { true,parse::GREATER_THAN };		// >
		}
		else if (pk.isValid() && (pk.sign_type == lex::S_EQUAL))
		{
			if (t.sign_type == lex::S_EQUAL)
			{
				E_LOG("EQUAL FOUND");
				pos_ += 1;
				return { true,parse::EQUAL };				// ==
			}
			else if (t.sign_type == lex::P_EXCL_MARK)
			{
				pos_ += 1;
				return { true,parse::NOTEQUAL };			// !=
			}
		}
	}
	E_LOG("Can't match prior op");
	pos_ -= 1; // found nothing, decrement and return;
	return { false, parse::optype::DEFAULT };
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

void Moonshot::Parser::errorExpected(const std::string & s)
{
	std::stringstream ss;
	ss << s << " after token (in position" << getToken().showFormattedTokenData() << ".)" << std::endl;
	E_ERROR(ss.str());
}
