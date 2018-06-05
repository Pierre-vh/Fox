////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Lexer.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Lexer.hpp"

#include <string>		// std::string
#include <cwctype>		// std::iswspace
#include <sstream>		// std::stringstream (sizeToStr())
#include <cassert>

#include "Moonshot/Fox/AST/ASTContext.hpp"
#include "Moonshot/Fox/Common/Context.hpp"
#include "Moonshot/Fox/Common/Exceptions.hpp"

using namespace Moonshot;
using namespace Moonshot::Dictionaries;

Lexer::Lexer(Context & curctxt, ASTContext &astctxt) : context_(curctxt), astcontext_(astctxt)
{

}

void Lexer::lexStr(const std::string& str)
{
	lexFile(context_.sourceManager.loadFromString(str));
}

void Lexer::lexFile(const FileID& file)
{
	assert(file && "INVALID FileID!");
	fID_ = file;
	manip.setStr(context_.sourceManager.getSourceForFID(fID_));

	manip.reset();
	cstate_ = DFAState::S_BASE;

	while(!manip.eof() && context_.isSafe())
		cycle();

	pushTok(); // Push the last Token found.
	runFinalChecks();
	if (context_.isSafe())
	{
		if (context_.flagsManager.isSet(FlagID::lexer_logTotalTokenCount))
		{
			std::stringstream ss;
			ss << "Lexing finished Successfully. Tokens found: " << result_.size();
			context_.logMessage(ss.str());
		}
	}
	context_.resetOrigin();
}

void Lexer::logAllTokens() const
{
	for (const Token &tok : result_)
		context_.logMessage(tok.showFormattedTokenData());
}


TokenVector & Lexer::getTokenVector()
{
	return result_; // return empty Token
}

std::size_t Lexer::resultSize() const
{
	return result_.size();
}

void Lexer::pushTok()
{
	if (curtok_ == "")	// Don't push empty tokens.
		return;

	// Create a SourceLoc for the begin loc
	SourceLoc sloc(fID_, currentTokenBeginIndex_);
	// Create the SourceRange of this token:
	SourceRange range(sloc, static_cast<SourceRange::offset_type>(curtok_.size() - 1));
	Token t(context_,astcontext_,curtok_,range);

	if (t)
		result_.push_back(t);
	else
		context_.reportError("(this error is a placeholder for : Invalid token found)");

	curtok_ = "";

	if (context_.flagsManager.isSet(FlagID::lexer_logOnPush)) {
		std::stringstream out;
		out << "Pushed token \"" << result_.back().getAsString() << '"';
		context_.logMessage(out.str());
	}

}

void Lexer::cycle()
{
	if (!context_.isSafe())
	{
		reportLexerError("Errors found : stopping lexing process.");
		return;
	}
	runStateFunc();					// execute appropriate function
}

void Lexer::runFinalChecks()
{
	if (context_.isSafe())
	{
		switch (cstate_)
		{
			case DFAState::S_STR:
				reportLexerError("A String literal was still not closed when end of file was reached");
				break;
			case DFAState::S_CHR:
				reportLexerError("A Char literal was still not closed when end of file was reached");
				break;
			case DFAState::S_MCOM:
				reportLexerError("A Multiline comment was not closed when end of file was reached");
				break;
		}
	}
}

void Lexer::markBeginningOfToken()
{
	currentTokenBeginIndex_ = manip.getIndexInBytes();
}

void Lexer::runStateFunc()
{
	switch (cstate_)
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
	const CharType pk = manip.peekNext();
	const CharType c = manip.getCurrentChar();	// current char

	if (curtok_.size() != 0)	// simple error checking : the Token should always be empty when we're in S_BASE.
	{
		throw Exceptions::lexer_critical_error("Current Token isn't empty in S_BASE, current Token :" + curtok_);
		return;
	}
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
		markBeginningOfToken();
		addToCurtok(eatChar());
		pushTok();
	}
	// HANDLE STRINGS AND CHARS
	else if (c == '\'')	// Delimiter?
	{
		markBeginningOfToken();
		addToCurtok(eatChar());
		dfa_goto(DFAState::S_CHR);
	}
	else if (c == '"')
	{
		markBeginningOfToken();
		addToCurtok(eatChar());
		dfa_goto(DFAState::S_STR);
	}
	// HANDLE IDs & Everything Else
	else
	{
		markBeginningOfToken();
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
		reportLexerError("Newline characters (\\n) in string literals are illegal. Token concerned:" + curtok_);
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
	if (eatChar() == '*' && manip.getCurrentChar() == '/')
	{
		eatChar();
		dfa_goto(DFAState::S_BASE);
	}
}

void Lexer::fn_S_WORDS()
{
	if (isSep(manip.getCurrentChar()))
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
			reportLexerError("Declared an empty char literal. Char literals must contain at least one character.");
	
		pushTok();
		dfa_goto(DFAState::S_BASE);
	}
	else if (c == '\n')
		reportLexerError("Newline characters (\\n) in char literals are illegal. Token concerned:" + curtok_);
	else
		addToCurtok(c);
}

void Lexer::dfa_goto(const DFAState & ns)
{
	cstate_ = ns;
}

CharType Lexer::eatChar()
{
	const CharType c = manip.getCurrentChar();
	manip.advance();
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

bool Lexer::isSep(const CharType &c) const
{
	// Is separator ? Signs are the separators in the input. Separators mark the end and beginning of tokens, and are tokens themselves. Examples : Hello.World -> 3 Tokens. "Hello", "." and "World."
	if (c == '.' && std::iswdigit(static_cast<wchar_t>(manip.peekNext()))) // if the next character is a digit, don't treat the dot as a separator.
		return false;
	// To detect if C is a sign separator, we use the sign dictionary
	auto i = kSign_dict.find(c);
	return (i != kSign_dict.end()) || std::iswspace((wchar_t)c);
}

bool Lexer::isEscapeChar(const CharType & c) const
{
	return  (c == '\\') && ((cstate_ == DFAState::S_STR) || (cstate_ == DFAState::S_CHR));
}

bool Lexer::shouldIgnore(const CharType & c) const
{
	return (c == '\r'); // don't push carriage returns
}

void Lexer::reportLexerError(std::string errmsg) const
{
	context_.reportError(errmsg);
}