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

// Note: Most of the code here is a relic of the old moonshot. It'll be completly rewritten in the not so distant future

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

			// Lexs a raw String.
			// This will load the string into the SourceManager for you, and returns the FileID.
			FileID lexStr(const std::string& str);

			// Lexs a file in the SourceManager.
			// This will retrieve the file from the SourceManager from the current Context.
			void lexFile(const FileID& file);
			
			void logAllTokens() const;
			TokenVector& getTokenVector();
			std::size_t resultSize() const;	

		private:
			enum class DFAState
			{
				S_BASE, // basestate
				S_STR,	// string literals
				S_LCOM,	// line comment
				S_MCOM,	// multiline comment
				S_WORDS,// basic (keywords,signs) state
				S_CHR	// char literals
			};

			void pushTok();
			void cycle();					
			void runFinalChecks();
			void markBeginningOfToken(); // sets currentTokenBeginIndex_ to the current index

			void runStateFunc();
			void dfa_goto(const DFAState &ns);
			void fn_S_BASE();	// base state
			void fn_S_STR();	// string literals
			void fn_S_LCOM();	// one line comment
			void fn_S_MCOM();	// Multiple line comments
			void fn_S_WORDS();	// "basic" state (push to tok until separator is met)
			void fn_S_CHR();	// Char literals

			// Utils
			CharType eatChar();									// returns the current char and run updatePos (returns inputstr_[pos_] and do pos_+=1)
			void addToCurtok(CharType c);						// adds the current character to curtok
			bool isSep(const CharType &c) const;				// is the current char a separator? (= a sign. see kSign_dict)
			bool isEscapeChar(const CharType& c) const;			// Checks if C is \ AND if the state is adequate for it to be qualified as an escape char.
			bool shouldIgnore(const CharType& c) const;			// Checks if the char is valid to be pushed. If it isn't and it should be ignored, returns true

			void reportLexerError(std::string errmsg) const;

			ASTContext &astContext_;
			Context& context_;
			FileID currentFile_;

			bool		escapeFlag_ = false;		
			DFAState	state_ = DFAState::S_BASE;		
			std::string curtok_;

			// The index of the first character of the current token being processed.
			SourceLoc::idx_type currentTokenBeginIndex_;

			TokenVector	tokens_;
			StringManipulator manip_;
	};
}
