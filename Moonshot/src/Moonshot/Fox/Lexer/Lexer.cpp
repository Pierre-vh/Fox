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

// TODO : The code is a bit messy : clean it up ! Maybe rework states and such ? But it works for now, so I keep it until a rework is needed (for performance reasons, or if i do a complete rewrite)

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
	cstate_ = dfa::S0;
	while(pos_ < data.size())
		cycle();
	E_LOG("Lexing finished. Tokens found: " + sizeToString(result_.size()))
}

void Moonshot::Lexer::iterateResults(std::function<void(const token&)> func)
{
	for (const token &tok : result_)
		func(tok);
}

void Moonshot::Lexer::logAllTokens() const
{
	for (const token &tok : result_)
		E_LOG(tok.showFormattedTokenData());
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
	// this line was used for debug purposes.
	//E_LOG("Pushing token \"" + curtok_ + "\"")
	token t(curtok_,ccoord_);
	result_.push_back(t);
	curtok_ = "";
}

void Lexer::cycle()
{
	// update position
	ccoord_.column += 1;
	if (str_[pos_] == '\n')
		ccoord_.newLine();
	// execute proper state's function
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
	char pk = peekNext();
	char c = str_[pos_];	// Get current char without advancing in the stream

	if (curtok_.size() != 0)	// simple error checking : the token should always be empty when we're in S0.
	{
		E_CRITICAL("ERROR. CURRENT TOKEN IS NOT EMPTY IN S0.");
		return;
	}
	// IGNORE SPACES
	if (std::isspace(c))
		forward();
	// HANDLE COMMENTS
	else if (c == '/' && pk == '/')
	{
		forward();
		dfa_goto(dfa::S2);
	}
	else if (c == '/' && pk == '*')
	{
		forward();
		dfa_goto(dfa::S3);
	}
	// HANDLE SINGLE SEPARATOR
	else if (isSep(c))				// is the current char a separator, but not a space?
	{
		addToCurtok(eatChar());
		pushTok();
	}
	// HANDLE STRINGS AND CHARS
	else if (c == '\'')	// Delimiter?
	{
		addToCurtok(eatChar());
		dfa_goto(dfa::S5);
	}
	else if (c == '"')
	{
		addToCurtok(eatChar());
		dfa_goto(dfa::S1);
	}
	// HANDLE IDs
	else 		
		dfa_goto(dfa::S4);

}

void Lexer::dfa_S1()
{
	char bck = curtok_.back();
	char c = eatChar();
	addToCurtok(c);
	if (c == '"' && bck != '\\')
	{
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
		forward();
		dfa_goto(dfa::S0);
	}
}

void Lexer::dfa_S4()
{
	if (isSep(str_[pos_]))
	{
		pushTok();
		dfa_goto(dfa::S0);
	}
	else 
		addToCurtok(eatChar());
}

void Moonshot::Lexer::dfa_S5()
{
	char c = eatChar();
	addToCurtok(c);
	if (c == '\'' && curtok_.back() != '\\')
	{
		pushTok();
		dfa_goto(dfa::S0);
	}
}

void Lexer::dfa_goto(const dfa::state & ns)
{
	cstate_ = ns;
}

char Moonshot::Lexer::eatChar()
{
	char c = str_[pos_];
	forward();
	return c;
}

char Lexer::peekNext(const size_t &p) const
{
	if (p+1 >= (str_.size()))	// checks if it's possible
		return ' ';
	return str_[p + 1];
}

void Moonshot::Lexer::addToCurtok(const char & c)
{
	if(curtok_.size() > 0)
	if (curtok_.back() == '\\')
	{
		switch (c)
		{
			case '\'':
			case '"':
				curtok_.pop_back();
				break;
		}
	}
	curtok_ += c;
}

bool Lexer::isSep(const char &c) const
{
	if (c == '.' && std::isdigit(peekNext()))	// if we're inside a number, we shouldn't treat a dot as a separator.
		return false;
	auto i = lex::kSign_dict.find(c);
	return i != lex::kSign_dict.end() || std::isspace(c);
}

char Lexer::peekNext() const
{
	return peekNext(pos_);
}

void Moonshot::Lexer::forward()
{
	pos_ += 1;
}

std::string Moonshot::Lexer::sizeToString(const size_t &s) const
{
	std::stringstream ss;
	ss << s;
	return ss.str();
}
