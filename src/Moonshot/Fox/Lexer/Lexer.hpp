////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Lexer.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the lexer class.						
//															
// The lexer is the 1st step of the interpretation process.
// It takes the source file, in the form of a string, as input, and outputs a std::<vector> of Token.				
//															
// It performs a lexical analysis. A Fairly simple one in our case, using a DFA. (state machine)
//															
//															
// Tokens (see Token.hpp/.cpp for declaration and definition is the dissected entry, separated in small bits			
// each "bit" is identified to recognize one of the main types : keywords,identifiers,values,etc..				
////------------------------------------------------------////

#pragma once

#include "Token.hpp"
#include "Moonshot/Fox/Common/StringManipulator.hpp"

namespace Moonshot
{
	class Context;

	class Lexer 
	{
		public:
			Lexer(Context& curctxt,ASTContext &astctxt);

			// input: data : a UTF-8 encoded std::string.
			void lexStr(const std::string &data);		// Main function.

			void logAllTokens() const;					// log all Tokens in output using context_.log . Useful for debugging.

			// get the result
			TokenVector& getTokenVector();				// returns the n th Token in result_
			// get the number of tokens in the result
			std::size_t resultSize() const;				// returns result_.size()
		
			void setStr(const std::string* str);
		private:
			// enum to keep track of the current state
			enum class DFAState
			{
				S_BASE, // basestate
				S_STR,	// string literals
				S_LCOM,	// line comment
				S_MCOM,	// multiline comment
				S_WORDS,// basic (keywords,signs) state
				S_CHR	// char literals
			};

			void pushTok();					// push Token
			void cycle();					// one dfa "cycle";
			void runFinalChecks();			// runs the final checks. this is called after the lexing process ended.
			// DFA state functions. I split this into various functions to make the code more readable in the cycle() function.
			void runStateFunc();			// call the correct function, depending on cstate_
			void dfa_goto(const DFAState &ns); 	// Go to state X (changes cstate)
			// States functions
			void fn_S_BASE();
			void fn_S_STR();	// string literals
			void fn_S_LCOM();	// one line comment
			void fn_S_MCOM();	// Multiple line comments
			void fn_S_WORDS();	// "basic" state (push to tok until separator is met)
			void fn_S_CHR();	// Char literals

			// Utils
			CharType eatChar();									// returns the current char and run updatePos (returns inputstr_[pos_] and do pos_+=1)
			void addToCurtok(CharType c);						// adds the current character to curtok_
			bool isSep(const CharType &c) const;				// is the current char a separator? (= a sign. see kSign_dict)
			bool isEscapeChar(const CharType& c) const;			// Checks if C is \ AND if the state is adequate for it to be qualified as an escape char.
			bool shouldIgnore(const CharType& c) const;			// Checks if the char is valid to be pushed. If it isn't and it should be ignored, returns true

			// error management made easy
			void reportLexerError(std::string errmsg) const;

			// Member Variables
			// ASTContext
			ASTContext &astcontext_;
			// Context
			Context& context_;
			// Utilities
			bool		escapeFlag_ = false;			// escaping with backslash flag
			DFAState	cstate_ = DFAState::S_BASE;		// curren dfa state. begins at S_BASE;
			std::string curtok_;					// the Token that's being constructed.
			TextPosition	ccoord_;					// current coordinates.
			// Output
			TokenVector	result_;		// the lexer's output !
			//
			StringManipulator manip;
	};
}
