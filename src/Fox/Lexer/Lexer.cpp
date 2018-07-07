////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Lexer.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Lexer.hpp"

#include <string>
#include <cwctype>
#include <sstream>		
#include <cassert>
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/AST/ASTContext.hpp"

using namespace fox;
using namespace fox::Dictionaries;

Lexer::Lexer(DiagnosticEngine& diags,SourceManager& sm, ASTContext &astctxt) : diags_(diags), astContext_(astctxt), sm_(sm)
{

}

void Lexer::lexFile(const FileID& file)
{
	assert(file && "INVALID FileID!");
	currentFile_ = file;
	manip_.setStr(sm_.getSourceForFID(currentFile_));

	manip_.reset();
	state_ = DFAState::S_BASE;

	while(!manip_.eof())
		cycle();

	pushTok();
	runFinalChecks();
}

TokenVector & Lexer::getTokenVector()
{
	return tokens_; // return empty Token
}

std::size_t Lexer::resultSize() const
{
	return tokens_.size();
}

FileID Lexer::getCurrentFile() const
{
	return currentFile_;
}

void Lexer::pushTok()
{
	if (curtok_ == "")	// Don't push empty tokens.
		return;

	// Create a SourceLoc for the begin loc
	SourceLoc sloc(currentFile_, currentTokenBeginIndex_);
	// Create the SourceRange of this token:
	Token t(diags_,astContext_,curtok_,getCurtokRange());

	if (t)
		tokens_.push_back(t);
	else
		diags_.report(DiagID::lexer_invalid_token_found,t.getRange()).addArg(t.getAsString());

	curtok_ = "";
}

void Lexer::cycle()
{
	runStateFunc();					
}

void Lexer::runFinalChecks()
{
	switch (state_)
	{
		case DFAState::S_STR:
			// FALL THROUGH
		case DFAState::S_CHR:
			diags_.report(DiagID::lexer_missing_closing_quote, getCurtokBegLoc());
			break;
		case DFAState::S_MCOM:
			diags_.report(DiagID::lexer_unfinished_multiline_comment, getCurtokBegLoc());
			break;
	}
}

void Lexer::markBeginningOfToken()
{
	currentTokenBeginIndex_ = manip_.getIndexInBytes();
}

void Lexer::runStateFunc()
{
	switch (state_)
	{
		case DFAState::S_BASE:
			fn_S_BASE();
			break;
		case DFAState::S_STR:
			fn_S_STR();
			break;
		case DFAState::S_LCOM:
			fn_S_LCOM();
			break;
		case DFAState::S_MCOM:
			fn_S_MCOM();
			break;
		case DFAState::S_WORDS:
			fn_S_WORDS();
			break;
		case DFAState::S_CHR:
			fn_S_CHR();
			break;
	}
}

void Lexer::fn_S_BASE()
{
	const CharType pk = manip_.peekNext();
	const CharType c = manip_.getCurrentChar();	// current char

	assert((curtok_.size() == 0) && "Curtok not empty in base state");

	markBeginningOfToken();

	// IGNORE SPACES
	if (std::iswspace(static_cast<wchar_t>(c))) eatChar();
	// HANDLE COMMENTS
	else if (c == '/' && pk == '/')
	{
		eatChar();
		dfa_goto(DFAState::S_LCOM);
	}
	else if (c == '/' && pk == '*')
	{
		eatChar();
		dfa_goto(DFAState::S_MCOM);
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
		dfa_goto(DFAState::S_CHR);
	}
	else if (c == '"')
	{
		addToCurtok(eatChar());
		dfa_goto(DFAState::S_STR);
	}
	// HANDLE IDs & Everything Else
	else
	{
		dfa_goto(DFAState::S_WORDS);
	}

}

void Lexer::fn_S_STR()
{
	CharType c = eatChar();
	if (c == '"' && !escapeFlag_)
	{
		addToCurtok(c);
		pushTok();
		dfa_goto(DFAState::S_BASE);
	}
	else if (c == '\n')
		diags_.report(DiagID::lexer_newline_in_literal, getCurtokBegLoc()).addArg("string");
	else
		addToCurtok(c);
}

void Lexer::fn_S_LCOM()				// One line comment state.
{
	if (eatChar() == '\n')			// Wait for new line
		dfa_goto(DFAState::S_BASE);			// then go back to S_BASE.
}

void Lexer::fn_S_MCOM()
{
	if (eatChar() == '*' && manip_.getCurrentChar() == '/')
	{
		eatChar();
		dfa_goto(DFAState::S_BASE);
	}
}

void Lexer::fn_S_WORDS()
{
	if (isSep(manip_.getCurrentChar()))
	{		
		pushTok();
		dfa_goto(DFAState::S_BASE);
	}
	else 
		addToCurtok(eatChar());
}

void Lexer::fn_S_CHR()
{
	CharType c = eatChar();
	if (c == '\'' && !escapeFlag_)
	{
		addToCurtok(c);

		if (curtok_.size() == 2)
			diags_.report(DiagID::lexer_empty_char_literal, getCurtokRange());

		pushTok();
		dfa_goto(DFAState::S_BASE);
	}
	else if (c == '\n')
		diags_.report(DiagID::lexer_newline_in_literal, getCurtokRange());
	else
		addToCurtok(c);
}

void Lexer::dfa_goto(DFAState ns)
{
	state_ = ns;
}

CharType Lexer::eatChar()
{
	const CharType c = manip_.getCurrentChar();
	manip_.advance();
	return c;
}

void Lexer::addToCurtok(CharType c)
{
	if (isEscapeChar(c) && !escapeFlag_)
	{
		StringManipulator::append(curtok_, c);
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
		StringManipulator::append(curtok_, c);
		escapeFlag_ = false;
	}
}

bool Lexer::isSep(CharType c) const
{
	// Is separator ? Signs are the separators in the input. Separators mark the end and beginning of tokens, and are tokens themselves. Examples : Hello.World -> 3 Tokens. "Hello", "." and "World."
	if (c == '.' && std::iswdigit(static_cast<wchar_t>(manip_.peekNext()))) // if the next character is a digit, don't treat the dot as a separator.
		return false;
	// To detect if C is a sign separator, we use the sign dictionary
	auto i = kSign_dict.find(c);
	return (i != kSign_dict.end()) || std::iswspace((wchar_t)c);
}

bool Lexer::isEscapeChar(CharType c) const
{
	return  (c == '\\') && ((state_ == DFAState::S_STR) || (state_ == DFAState::S_CHR));
}

bool Lexer::shouldIgnore(CharType c) const
{
	return (c == '\r'); // don't push carriage returns
}

SourceLoc Lexer::getCurtokBegLoc() const
{
	return SourceLoc(currentFile_,currentTokenBeginIndex_);
}

SourceRange Lexer::getCurtokRange() const
{
	return SourceRange(getCurtokBegLoc(), static_cast<SourceRange::offset_type>(curtok_.size() - 1));
}