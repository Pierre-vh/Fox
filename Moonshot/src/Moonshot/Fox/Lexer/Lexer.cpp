/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : See header.

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

#include "Lexer.h"

using namespace Moonshot;

Lexer::Lexer()
{

}

Lexer::~Lexer()
{
}

void Lexer::lexStr(const std::string & data)
{
	str_ = data;
	pos_ = 0;
	while(pos_ < data.size())
		cycle();
}

token Lexer::getToken(const size_t & vtpos)
{
	if (vtpos < result_.size())
		return result_[vtpos];
	E_CRITICAL("Tried to access a position in result_ that was out of bounds.")
}

size_t Lexer::resultSize()
{
	return result_.size();
}

void Lexer::pushTok()
{
	token t(curtok_);
	result_.push_back(t);
	curtok_ = "";
}

void Lexer::cycle()
{
	switch (cstate_)
	{
		case dfa::S0:
			dfa_S0();
			break;
		case dfa::S1:
			dfa_S1();
			break;
		case dfa::S2:
			dfa_S2();
			break;
		case dfa::S3:
			dfa_S3();
			break;
		case dfa::S4:
			dfa_S4();
			break;
	}
}

void Lexer::dfa_S0()
{
	char c = eatChar();
	if (curtok_.size() != 0)	// simple error checking : the token should always be empty when we're in S0.
	{
		E_CRITICAL("ERROR. CURRENT TOKEN IS NOT EMPTY IN S0.");
		return;
	}

	if (c == '/' && str_[pos_] == '/')
	{
		pos_ += 1;					// update position
		dfa_goto(dfa::S2);
	}
	else if (c == '/' && str_[pos_] == '*')
	{
		pos_ += 1;
		dfa_goto(dfa::S3);
	}
	else if (isSep(c))				// is the current char a separator?
	{
		addToCurtok(c);
		pushTok();
	}
	else if (c == '\'' || c == '"')	// Delimiter?
	{
		addToCurtok(c);
		dfa_goto(dfa::S1);
	}
	else							// else, we just assume it's a a-z / A-Z / _ character. if it's not, the token::selfId() method handle the error.
	{
		addToCurtok(c);
		dfa_goto(dfa::S4);
	}
}

void Lexer::dfa_S1()
{
	char c = eatChar();
	if ((c == '"' || c == '\'') && !isEscaped(pos_-1))
	{
		addToCurtok(c);
		pushTok();
		dfa_goto(dfa::S0);
	}
}

void Lexer::dfa_S2()				// One line comment state.
{
	if (eatChar() == '\n')			// Wait for new line
		dfa_goto(dfa::S0);			// then go back to S0.
}

void Lexer::dfa_S3()
{
	if (eatChar() == '*' && str_[pos_] == '/')
	{
		pos_ += 1;
		dfa_goto(dfa::S0);
	}
}

void Lexer::dfa_S4()
{
	if (isSep(peekNext()))
	{
		addToCurtok(eatChar());
		pushTok();
		dfa_goto(dfa::S0);
	}
	else
		addToCurtok(eatChar());
}

void Lexer::dfa_goto(const dfa::state & ns)
{
	cstate_ = ns;
}

char Moonshot::Lexer::eatChar()
{
	char c = str_[pos_];
	pos_ += 1;
	return c;
}

char Lexer::peekNext(const size_t &p) const
{
	if (p < (str_.size() - 1))	// checks if it's possible
		return ' ';
	return str_[p + 1];
}

bool Lexer::isEscaped(const size_t &p) const
{
	if (p != 0)	// checks if we're not on the first character
		if (str_[p - 1] == '\\')
			return true;
	return false;
}

void Moonshot::Lexer::addToCurtok(const char & c)
{
	if (!std::isspace(c))
		curtok_ += c;
}

bool Lexer::isSep(const char &c) const
{
	auto i = lex::kSign_dict.find(c);
	return (i != lex::kSign_dict.end()) || (c == ' ');
}

bool Lexer::isSpace(const size_t &p) const
{
	return (str_[p] == ' ');
}

char Lexer::peekNext() const
{
	return peekNext(pos_);
}

bool Lexer::isEscaped() const
{
	return isEscaped(pos_);
}

bool Lexer::isSpace() const
{
	return isSpace(pos_);
}

