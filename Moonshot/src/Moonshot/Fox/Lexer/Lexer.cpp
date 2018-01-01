////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Lexer.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

// Note : For now the "escape" char system is working nicely, but the way i've done it is less than ideal.
// Task : rework it, find a cleaner way to do it, or at least do a cleanup of the code !

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
	E_SET_ERROR_CONTEXT("LEXING");

	str_ = data;
	pos_ = 0;
	cstate_ = dfa::S0;
	while(pos_ < data.size() && E_CHECKSTATE)
		cycle();
	if(curtok_ != "")
		pushTok(); // Push the last token formed, if it's not empty.

	if ((cstate_ == dfa::S1 || cstate_ == dfa::S5) && E_CHECKSTATE) // If we were in the middle of lexing a string/char
		reportLexerError("Met the end of the file before a closing delimiter for char/strings");

	if constexpr (LOG_TOTALTOKENSCOUNT)
		E_LOG("Lexing finished. Tokens found: " + sizeToString(result_.size()));

	E_RESET_ERROR_CONTEXT;
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

token Lexer::getToken(const size_t & vtpos) const
{
	if (vtpos < result_.size())
		return result_[vtpos];
	E_CRITICAL("Tried to access a position in result_ that was out of bounds.");
	return token();
}

size_t Lexer::resultSize() const
{
	return result_.size();
}

void Lexer::pushTok()
{
	if(LOG_PUSHEDTOKENS)
		std::cout << "Pushing token <" << curtok_ << ">"  << std::endl;
	token t(curtok_,ccoord_);
	result_.push_back(t);
	curtok_ = "";
}

void Lexer::cycle()
{
	if (!E_CHECKSTATE)
	{
		reportLexerError("Errors found : stopping lexing process.");
		return;
	}
	// update position
	ccoord_.forward();
	// execute appropriate function
	auto it = kState_dict.find(cstate_);
	if (it != kState_dict.end())
	{
		auto fn = it->second;
		fn(*this);
	}
	// update line
	if (str_[pos_] == '\n')
	{
		ccoord_.newLine();
	}
}

void Lexer::dfa_S0()
{
	char pk = peekNext();
	char c = str_[pos_];	// Get current char without advancing in the stream

	if (curtok_.size() != 0)	// simple error checking : the token should always be empty when we're in S0.
	{
		E_CRITICAL("ERROR. CURRENT TOKEN IS NOT EMPTY IN S0. TOKEN IS:" + curtok_);
		return;
	}
	// IGNORE SPACES
	if (std::iswspace(c)) forward();
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
	// HANDLE IDs & Everything Else
	else 		
		dfa_goto(dfa::S4);

}

void Lexer::dfa_S1()
{
	char c = eatChar();
	if (c == '"' && !escapes_)
	{
		addToCurtok(c);
		pushTok();
		dfa_goto(dfa::S0);
	}
	else if (c == '\n')
		reportLexerError("Newline characters (\\n) in string values declarations are illegal.\nToken concerned:" + curtok_);
	else
		addToCurtok(c);
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

void Lexer::dfa_S5()
{
	char c = eatChar();
	if (c == '\'' && !escapes_)
	{
		addToCurtok(c);
		pushTok();
		dfa_goto(dfa::S0);
	}
	else if (c == '\n')
		reportLexerError("Newline characters (\\n) in char values declarations are illegal.\nToken concerned:" + curtok_);
	else
		addToCurtok(c);
}

void Lexer::dfa_goto(const dfa::state & ns)
{
	cstate_ = ns;
}

char Lexer::eatChar()
{
	const char c = str_[pos_];
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
	if (c == '\\' && ((cstate_ == dfa::S1)||(cstate_ == dfa::S5)))
	{
		if (escapes_)
		{
			curtok_ += c;
			escapes_ = false;
		}
		else
			escapes_ = true;
	}
	else
	{
		curtok_ += c;
		escapes_ = false;
	}
}

bool Lexer::isSep(const char &c) const
{
	if (c == '.' && std::iswdigit(peekNext()))	// if we're inside a number, we shouldn't treat a dot as a separator.
		return false;
	auto i = lex::kSign_dict.find(c);
	return i != lex::kSign_dict.end() || std::iswspace(c);
}

char Lexer::peekNext() const
{
	return peekNext(pos_);
}

void Moonshot::Lexer::forward()
{
	pos_ += 1;
}

void Lexer::reportLexerError(const std::string & errmsg) const
{
	std::stringstream out;
	out << errmsg << " at line " << ccoord_.line << std::endl; // Somehow I have to use line-1 to get the correct line count.
	E_ERROR(out.str());
}

std::string Lexer::sizeToString(const size_t &s) const
{
	std::stringstream ss;
	ss << s;
	return ss.str();
}
