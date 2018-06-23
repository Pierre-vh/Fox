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

Lexer::Lexer(Context & curctxt, ASTContext &astctxt) : context(curctxt), astcontext(astctxt)
{

}

FileID Lexer::lexStr(const std::string& str)
{
	auto fid = context.sourceManager.loadFromString(str);
	if(fid)
		lexFile(fid);
	return fid;
}

void Lexer::lexFile(const FileID& file)
{
	assert(file && "INVALID FileID!");
	currentFile = file;
	stringManipulator.setStr(context.sourceManager.getSourceForFID(currentFile));

	stringManipulator.reset();
	state = DFAState::S_BASE;

	while(!stringManipulator.eof() && context.isSafe())
		cycle();

	pushTok();
	runFinalChecks();
	context.resetOrigin();
}

void Lexer::logAllTokens() const
{
	for (const Token &tok : tokens)
		context.logMessage(tok.showFormattedTokenData());
}


TokenVector & Lexer::getTokenVector()
{
	return tokens; // return empty Token
}

std::size_t Lexer::resultSize() const
{
	return tokens.size();
}

void Lexer::pushTok()
{
	if (curtok == "")	// Don't push empty tokens.
		return;

	// Create a SourceLoc for the begin loc
	SourceLoc sloc(currentFile, currentTokenBeginIndex);
	// Create the SourceRange of this token:
	SourceRange range(sloc, static_cast<SourceRange::offset_type>(curtok.size() - 1));
	Token t(context,astcontext,curtok,range);

	if (t)
		tokens.push_back(t);
	else
		context.reportError("(this error is a placeholder for : Invalid token found)");

	curtok = "";
}

void Lexer::cycle()
{
	if (!context.isSafe())
	{
		reportLexerError("Errors found : stopping lexing process.");
		return;
	}
	runStateFunc();					// execute appropriate function
}

void Lexer::runFinalChecks()
{
	if (context.isSafe())
	{
		switch (state)
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
	currentTokenBeginIndex = stringManipulator.getIndexInBytes();
}

void Lexer::runStateFunc()
{
	switch (state)
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
	const CharType pk = stringManipulator.peekNext();
	const CharType c = stringManipulator.getCurrentChar();	// current char

	if (curtok.size() != 0)	// simple error checking : the Token should always be empty when we're in S_BASE.
	{
		throw Exceptions::lexer_critical_error("Current Token isn't empty in S_BASE, current Token :" + curtok);
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
	if (c == '"' && !escapeFlag)
	{
		addToCurtok(c);
		pushTok();
		dfa_goto(DFAState::S_BASE);
	}
	else if (c == '\n')
		reportLexerError("Newline characters (\\n) in string literals are illegal. Token concerned:" + curtok);
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
	if (eatChar() == '*' && stringManipulator.getCurrentChar() == '/')
	{
		eatChar();
		dfa_goto(DFAState::S_BASE);
	}
}

void Lexer::fn_S_WORDS()
{
	if (isSep(stringManipulator.getCurrentChar()))
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
	if (c == '\'' && !escapeFlag)
	{
		addToCurtok(c);

		if (curtok.size() == 2)
			reportLexerError("Declared an empty char literal. Char literals must contain at least one character.");
	
		pushTok();
		dfa_goto(DFAState::S_BASE);
	}
	else if (c == '\n')
		reportLexerError("Newline characters (\\n) in char literals are illegal. Token concerned:" + curtok);
	else
		addToCurtok(c);
}

void Lexer::dfa_goto(const DFAState & ns)
{
	state = ns;
}

CharType Lexer::eatChar()
{
	const CharType c = stringManipulator.getCurrentChar();
	stringManipulator.advance();
	return c;
}

void Lexer::addToCurtok(CharType c)
{
	if (isEscapeChar(c) && !escapeFlag)
	{
		StringManipulator::append(curtok, c);
		escapeFlag = true;
	}
	else if(!shouldIgnore(c))
	{
		if (escapeFlag)	// last char was an escape char
		{
			switch (c)
			{
				case 't':
					c = '\t';
					curtok.pop_back();
					break;
				case 'n':
					c = '\n';
					curtok.pop_back();
					break;
				case 'r':
					curtok.pop_back();
					c = '\r';
					break;
				case '\\':
				case '\'':
				case '"':
					curtok.pop_back();
					break;
			}

		}
		StringManipulator::append(curtok, c);
		escapeFlag = false;
	}
}

bool Lexer::isSep(const CharType &c) const
{
	// Is separator ? Signs are the separators in the input. Separators mark the end and beginning of tokens, and are tokens themselves. Examples : Hello.World -> 3 Tokens. "Hello", "." and "World."
	if (c == '.' && std::iswdigit(static_cast<wchar_t>(stringManipulator.peekNext()))) // if the next character is a digit, don't treat the dot as a separator.
		return false;
	// To detect if C is a sign separator, we use the sign dictionary
	auto i = kSign_dict.find(c);
	return (i != kSign_dict.end()) || std::iswspace((wchar_t)c);
}

bool Lexer::isEscapeChar(const CharType & c) const
{
	return  (c == '\\') && ((state == DFAState::S_STR) || (state == DFAState::S_CHR));
}

bool Lexer::shouldIgnore(const CharType & c) const
{
	return (c == '\r'); // don't push carriage returns
}

void Lexer::reportLexerError(std::string errmsg) const
{
	context.reportError(errmsg);
}