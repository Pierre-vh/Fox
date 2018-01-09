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


Lexer::Lexer(Context & curctxt) : context_(curctxt)
{
}

Lexer::~Lexer()
{
}

void Lexer::lexStr(const std::string & data)
{
	context_.setOrigin("LEXER");

	inputstr_ = data;
	pos_ = 0;
	cstate_ = dfaState::S0;
	while(pos_ < data.size() && context_.isSafe())
		cycle();

	pushTok(); // Push the last token found.

	if (options.logTotalTokensCount)
		context_.logMessage("Lexing finished. Tokens found: " + (int)result_.size());

	context_.resetOrigin();
}

void Lexer::iterateResults(std::function<void(const token&)> func)
{
	for (const token &tok : result_)
		func(tok);
}

void Lexer::logAllTokens() const
{
	for (const token &tok : result_)
		context_.logMessage(tok.showFormattedTokenData());
}

token Lexer::getToken(const std::size_t & vtpos) const
{
	if (vtpos < result_.size())
		return result_[vtpos];

	throw std::out_of_range("Tried to access a position in result_ that was out of bounds.");
	return token(context_); // return empty token
}

std::size_t Lexer::resultSize() const
{
	return result_.size();
}

void Lexer::pushTok()
{
	if (options.logPushedTokens) {
		std::stringstream out;
		out << "Pushing token \xAE" + curtok_ + "\xAF";
		context_.logMessage(out.str());
	}

	if (curtok_ == "")	// Don't push empty tokens.
		return;

	// push token
	token t(context_,curtok_,ccoord_);
	result_.push_back(t);
	curtok_ = "";
}

void Lexer::cycle()
{
	if (!context_.isSafe())
	{
		reportLexerError("Errors found : stopping lexing process.");
		return;
	}
	ccoord_.forward();				// update position
	runStateFunc();					// execute appropriate function
	if (inputstr_[pos_] == '\n')	// update line
	{
		ccoord_.newLine();
	}
}

void Lexer::runFinalChecks()
{
	if ((cstate_ == dfaState::S1 || cstate_ == dfaState::S5) && context_.isSafe()) // If we were in the middle of lexing a string/char
		reportLexerError("Met the end of the file before a closing delimiter for char/strings");
}

void Lexer::runStateFunc()
{
	switch (cstate_)
	{
		case dfaState::S0:
			dfa_S0();
			break;
		case dfaState::S1:
			dfa_S1();
			break;
		case dfaState::S2:
			dfa_S2();
			break;
		case dfaState::S3:
			dfa_S3();
			break;
		case dfaState::S4:
			dfa_S4();
			break;
		case dfaState::S5:
			dfa_S5();
			break;
	}
}

void Lexer::dfa_S0()
{
	char pk = peekNext();
	char c = inputstr_[pos_];	// Get current char without advancing in the stream

	if (curtok_.size() != 0)	// simple error checking : the token should always be empty when we're in S0.
	{
		throw Exceptions::lexer_critical_error("Current token isn't empty in S0, current token :" + curtok_);
		return;
	}
	// IGNORE SPACES
	if (std::iswspace(c)) eatChar();
	// HANDLE COMMENTS
	else if (c == '/' && pk == '/')
	{
		eatChar();
		dfa_goto(dfaState::S2);
	}
	else if (c == '/' && pk == '*')
	{
		eatChar();
		dfa_goto(dfaState::S3);
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
		dfa_goto(dfaState::S5);
	}
	else if (c == '"')
	{
		addToCurtok(eatChar());
		dfa_goto(dfaState::S1);
	}
	// HANDLE IDs & Everything Else
	else 		
		dfa_goto(dfaState::S4);

}

void Lexer::dfa_S1()
{
	char c = eatChar();
	if (c == '"' && !escapeFlag_)
	{
		addToCurtok(c);
		pushTok();
		dfa_goto(dfaState::S0);
	}
	else if (c == '\n')
		reportLexerError("Newline characters (\\n) in string values declarations are illegal. Token concerned:" + curtok_);
	else
		addToCurtok(c);
}

void Lexer::dfa_S2()				// One line comment state.
{
	if (eatChar() == '\n')			// Wait for new line
		dfa_goto(dfaState::S0);			// then go back to S0.
}

void Lexer::dfa_S3()
{
	if (eatChar() == '*' && inputstr_[pos_] == '/')
	{
		eatChar();
		dfa_goto(dfaState::S0);
	}
}

void Lexer::dfa_S4()
{
	if (isSep(inputstr_[pos_]))
	{		
		pushTok();
		dfa_goto(dfaState::S0);
	}
	else 
		addToCurtok(eatChar());
}

void Lexer::dfa_S5()
{
	char c = eatChar();
	if (c == '\'' && !escapeFlag_)
	{
		addToCurtok(c);
		pushTok();
		dfa_goto(dfaState::S0);
	}
	else if (c == '\n')
		reportLexerError("Newline characters (\\n) in char values declarations are illegal. Token concerned:" + curtok_);
	else
		addToCurtok(c);
}

void Lexer::dfa_goto(const dfaState & ns)
{
	cstate_ = ns;
}

char Lexer::eatChar()
{
	const char c = inputstr_[pos_];
	pos_ += 1;
	return c;
}

void Lexer::addToCurtok(const char & c)
{
	if (isEscapeChar(c) && !escapeFlag_)
	{
		curtok_ += c;
		escapeFlag_ = true;
	}
	else if(!shouldIgnore(c))
	{
		if (escapeFlag_)	// last char was an escape char
		{
			switch (c)
			{
				// In case we want to only have the escaped char, and not the backslash too
				case '\\':
				case '\'':
				case '"':
					curtok_.pop_back();
					break;
			}
		}
		curtok_ += c;
		escapeFlag_ = false;
	}
}

bool Lexer::isSep(const char &c) const
{
	if (c == '.' && std::iswdigit(peekNext()))	// if we're inside a number, we shouldn't treat a dot as a separator.
		return false;
	auto i = kSign_dict.find(c);
	return i != kSign_dict.end() || std::iswspace(c);
}

char Lexer::peekNext() const
{
	if (pos_ + 1 >= (inputstr_.size()))	// checks if it's possible
		return '\0';
	return inputstr_[pos_ + 1];
}

bool Lexer::isEscapeChar(const char & c) const
{
	return  (c == '\\') && ((cstate_ == dfaState::S1) || (cstate_ == dfaState::S5));
}

bool Lexer::shouldIgnore(const char & c) const
{
	return (c == '\r'); // don't push carriage returns
}

void Lexer::reportLexerError(std::string errmsg) const
{
	std::stringstream out;
	out << errmsg << " at line " << ccoord_.line; // Somehow I have to use line-1 to get the correct line count.
	context_.reportError(out.str());
}

