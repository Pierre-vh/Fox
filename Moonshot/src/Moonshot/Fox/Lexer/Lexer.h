////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Lexer.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the lexer class.						
//															
// The lexer is the 1st step of the interpretation process.
// It takes the source file, in the form of a string, as input, and outputs a std::<vector> of token.				
//															
// It performs a lexical analysis. A Fairly simple one in our case.
//															
//															
// Tokens (see Token.h/.cpp for declaration and definition is the dissected entry, separated in small bits			
// each "bit" is identified to recognize one of the main types : keywords,identifiers,values,etc..				
////------------------------------------------------------////

#pragma once

#include <string>		// std::string
#include <cwctype>		// std::iswspace
#include <vector>		// std::vector
#include <functional>	// std::function
#include <sstream>		// std::stringstream (sizeToStr())
#include <map>			// std::map
#include <memory>

#include "../../Common/Context/Context.h"
#include "../../Common/Exceptions/Exceptions.h"

#include "Token.h"
#include "../../Common/Options.h"


namespace Moonshot
{
	enum class dfaState
	{
		S0, S1, S2, S3, S4, S5	// the dfa's state.
	};
	class Lexer 
	{
		public:
			Lexer(Context& curctxt);
			~Lexer();

			void lexStr(const std::string &data);		// Main function.
			
			void iterateResults(std::function<void(const token&)> func);	// Some function that could be useful one day : takes a lambda with a token as argument.
																			// The lambda will then be called with each token of the output, in order.
			void logAllTokens() const;					// log all token using E_LOG. Useful for debugging.

			token getToken(const size_t &vtpos) const;	// returns the n th token in result_
			size_t resultSize() const;					// returns result_.size()
		private:
			// Context
			Context& context_;

			void pushTok();					// push token
			void cycle();					// one dfa "cycle";
			// DFA state functions. I split this into various functions to make the code more readable in the cycle() function.
			void dfa_S0();
			void dfa_S1();
			void dfa_S2();
			void dfa_S3();
			void dfa_S4();
			void dfa_S5();
			void dfa_goto(const dfaState &ns); 	// Go to state X

			// Useful functions 
			char eatChar();										// returns the current char and go forward in the stream (returns str_[pos_] and do pos_+=1)
			void addToCurtok(const char &c);					// adds the current character to curtok_, except if(isspace())
			bool isSep(const char &c) const;					// is the current char a separator? (= a sign. see kSign_dict)
			char peekNext(const size_t &p) const;				//	returns the next char after pos p 
			// Overloads with no arguments (will assume p = pos_)
			char peekNext() const;

			// This function's job is to increment pos_. Why use it ? Better readability in the code.
			void forward();

			// error management
			void reportLexerError(const std::string& errmsg) const;

			// Member Variables

			// dfa function dictionary : enum -> function
			const std::map<dfaState, std::function<void(Lexer &)>> kState_dict =
			{ 
				{	dfaState::S0	,	&Lexer::dfa_S0 },
				{	dfaState::S1	,	&Lexer::dfa_S1 },
				{	dfaState::S2	,	&Lexer::dfa_S2 },
				{	dfaState::S3	,	&Lexer::dfa_S3 },
				{	dfaState::S4	,	&Lexer::dfa_S4 },
				{	dfaState::S5	,	&Lexer::dfa_S5 }
			};
			//size_t to std::string
			std::string sizeToString(const size_t &s) const;

			// member variables
			bool escapes_ = false;				// escaping with backslash
			dfaState cstate_ = dfaState::S0;		// curren dfa state. begins at S0;
			std::string str_;					// the input
			size_t pos_ = 0;					// position in the input string;
			std::string curtok_;				// the token that's being constructed.
			text_pos ccoord_;					// current coordinates.
			std::vector<token>	result_;		// the lexer's output !
	};
}
