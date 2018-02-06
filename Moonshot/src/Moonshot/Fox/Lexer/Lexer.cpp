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
	cstate_ = dfaState::S_BASE;
	while(pos_ < data.size() && context_.isSafe())
		cycle();

	pushTok(); // Push the last Token found.

	if (context_.options.getAttr(OptionsList::lexer_logTotalTokenCount).value_or(false).get<bool>())
	{
		std::stringstream ss;
		ss << "Lexing finished. Tokens found: " << result_.size();
		context_.logMessage(ss.str());
	}
	context_.resetOrigin();
}

void Lexer::iterateResults(std::function<void(const Token&)> func)
{
	for (const Token &tok : result_)
		func(tok);
}

void Lexer::logAllTokens() const
{
	for (const Token &tok : result_)
		context_.logMessage(tok.showFormattedTokenData());
}


TokenVector & Moonshot::Lexer::getTokenVector()
{
	return result_; // return empty Token
}

std::size_t Lexer::resultSize() const
{
	return result_.size();
}

void Lexer::pushTok()
{
	if (context_.options.getAttr(OptionsList::lexer_logPushedTokens).value_or(false).get<bool>()) {
		std::stringstream out;
		out << "Pushing Token \xAE" + curtok_ + "\xAF";
		context_.logMessage(out.str());
	}

	if (curtok_ == "")	// Don't push empty tokens.
		return;

	// push Token
	Token t(context_,curtok_,ccoord_);
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
	if ((cstate_ == dfaState::S_STR || cstate_ == dfaState::S_CHR) && context_.isSafe()) // If we were in the middle of lexing a string/char
		reportLexerError("Met the end of the file before a closing delimiter for char/strings");
}

void Lexer::runStateFunc()
{
	switch (cstate_)
	{
		case dfaState::S_BASE:
			fn_S_BASE();
			break;
		case dfaState::S_STR:
			fn_S_STR();
			break;
		case dfaState::S_LCOM:
			fn_S_LCOM();
			break;
		case dfaState::S_MCOM:
			fn_S_MCOM();
			break;
		case dfaState::S_WORDS:
			fn_S_WORDS();
			break;
		case dfaState::S_CHR:
			fn_S_CHR();
			break;
	}
}

void Lexer::fn_S_BASE()
{
	char pk = peekNext();
	char c = inputstr_[pos_];	// Get current char without advancing in the stream

	if (curtok_.size() != 0)	// simple error checking : the Token should always be empty when we're in S_BASE.
	{
		throw Exceptions::lexer_critical_error("Current Token isn't empty in S_BASE, current Token :" + curtok_);
		return;
	}
	// IGNORE SPACES
	if (std::iswspace(c)) eatChar();
	// HANDLE COMMENTS
	else if (c == '/' && pk == '/')
	{
		eatChar();
		dfa_goto(dfaState::S_LCOM);
	}
	else if (c == '/' && pk == '*')
	{
		eatChar();
		dfa_goto(dfaState::S_MCOM);
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
		dfa_goto(dfaState::S_CHR);
	}
	else if (c == '"')
	{
		addToCurtok(eatChar());
		dfa_goto(dfaState::S_STR);
	}
	// HANDLE IDs & Everything Else
	else 		
		dfa_goto(dfaState::S_WORDS);

}

void Lexer::fn_S_STR()
{
	char c = eatChar();
	if (c == '"' && !escapeFlag_)
	{
		addToCurtok(c);
		pushTok();
		dfa_goto(dfaState::S_BASE);
	}
	else if (c == '\n')
		reportLexerError("Newline characters (\\n) in string literals are illegal. Token concerned:" + curtok_);
	else
		addToCurtok(c);
}

void Lexer::fn_S_LCOM()				// One line comment state.
{
	if (eatChar() == '\n')			// Wait for new line
		dfa_goto(dfaState::S_BASE);			// then go back to S_BASE.
}

void Lexer::fn_S_MCOM()
{
	if (eatChar() == '*' && inputstr_[pos_] == '/')
	{
		eatChar();
		dfa_goto(dfaState::S_BASE);
	}
}

void Lexer::fn_S_WORDS()
{
	if (isSep(inputstr_[pos_]))
	{		
		pushTok();
		dfa_goto(dfaState::S_BASE);
	}
	else 
		addToCurtok(eatChar());
}

void Lexer::fn_S_CHR()
{
	char c = eatChar();
	if (c == '\'' && !escapeFlag_)
	{
		addToCurtok(c);

		if (curtok_.size() == 2)
			reportLexerError("Declared an empty char literal. Char literals must contain at least one character.");
	
		pushTok();
		dfa_goto(dfaState::S_BASE);
	}
	else if (c == '\n')
		reportLexerError("Newline characters (\\n) in char literals are illegal. Token concerned:" + curtok_);
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

void Lexer::addToCurtok(char c)
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
				case 't':
					c = '\t';
					curtok_.pop_back();
					break;
				case 'n':
					c = '\n';
					curtok_.pop_back();
					break;
				case 'r':
					curtok_.pop_back();
					c = '\r';
					break;
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
	return  (c == '\\') && ((cstate_ == dfaState::S_STR) || (cstate_ == dfaState::S_CHR));
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