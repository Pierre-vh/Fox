#pragma once

#include <string>		// std::string
#include <cwctype>		// std::iswspace
#include <vector>		// std::vector
#include <functional>	// std::function
#include <sstream>		// std::stringstream (sizeToStr())
#include <map>			// std::map
#include "../../Common/Errors/Errors.h"
#include "Token.h"

// Used only for debugging purposes.
#define LOG_PUSHEDTOKENS false
#define LOG_TOTALTOKENSCOUNT false

namespace Moonshot
{
	namespace dfa
	{
		enum state
		{
			S0, S1, S2, S3, S4, S5	// the dfa's state.
		};
	}
	class Lexer
	{
		public:
			Lexer();
			~Lexer();

			void lexStr(const std::string &data);
			
			void iterateResults(std::function<void(const token&)> func);
			void logAllTokens() const;

			token getToken(const size_t &vtpos) const;	// returns the n th token in result_
			size_t resultSize() const;					// returns result_.size()
		private:
			// Private Methods

			// push a token to result_
			void pushTok();
			// a dfa cycle
			void cycle();					// one dfa "cycle";
			// DFA state behaviours. I split this into various functions to make the code more readable in the cycle() function.
			void dfa_S0();
			void dfa_S1();
			void dfa_S2();
			void dfa_S3();
			void dfa_S4();
			void dfa_S5();

			// Go to another state
			void dfa_goto(const dfa::state &ns);
			// Useful functions used in the lexer
			char eatChar();										// returns the current char and go forward in the stream (returns str_[pos_] and do pos_+=1)
			void addToCurtok(const char &c);					// adds the current character to curtok_, except if(isspace())
			bool isSep(const char &c) const;					// is the current char a separator? (= a sign. see lex::kSign_dict)
			char peekNext(const size_t &p) const;				//	returns the next char after pos p 
			// Overloads with no arguments (will assume p = pos_)
			char peekNext() const;

			// This function's job is to increment pos_. Why use it ? Better readability in the code.
			void forward();

			// Member Variables

			// dfa function dictionary
			const std::map<dfa::state, std::function<void(Lexer &)>> kState_dict =
			{ 
				{	dfa::S0	,	&Lexer::dfa_S0 },
				{	dfa::S1	,	&Lexer::dfa_S1 },
				{	dfa::S2	,	&Lexer::dfa_S2 },
				{	dfa::S3	,	&Lexer::dfa_S3 },
				{	dfa::S4	,	&Lexer::dfa_S4 },
				{	dfa::S5	,	&Lexer::dfa_S5 }
			};
			//size_t to std::string
			std::string sizeToString(const size_t &s) const;
			// member variables
			bool escapes_ = false;				// escaping with backslash
			dfa::state cstate_ = dfa::S0;		// curren dfa state. begins at S0;
			std::string str_;					// the input
			size_t pos_ = 0;					// position in the input.
			std::string curtok_;				// the token that's being constructed.
			text_pos ccoord_ = text_pos(1,0);	// current coordinates.
			std::vector<token>	result_;		// the lexer's output !
	};
}
