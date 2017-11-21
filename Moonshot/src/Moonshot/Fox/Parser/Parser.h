
/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Description : Parser class ! Here, every grammar rule is translated in a function ! 

*************************************************************
MIT License

Copyright (c) 2017 Pierre van Houtryve

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*************************************************************/
/*
	matchXXX functions : Parse a NON-TERMINAL.
		Theses function DON'T update the cursor BUT the cursor will be updated if the nonterminal is found, because the parseXXX function called will update it.
	parseXXX functions : Parse a TERMINAL
		Theses functions UPDATE the cursor.

	HOW TO IMPLEMENT A GRAMMAR RULE:
		1 - Check for the terminal or nonterminals of the rule
			Case a : Matched all token 
				Update the cursor, return a valid pointer
			Case b : Matched no token
				Don't update the cursor, return a null pointer
			Case c : matched one or more token, but encountered an unexpected token
				Don't update the cursor, return a null pointer, throw an error (with Error::reportError)

*/
#pragma once
// Lexer
#include "../Lexer/Lexer.h"
// Error reporting
#include "../../Common/Errors/Errors.h"
// AST
#include "AST\ASTNode.h"
#include "AST\ASTExpr.h"

#include <tuple>	// std::tuple, std::pair

namespace Moonshot
{
	class Parser
	{
		public:
			Parser(Lexer *l);
			~Parser();

			ASTNode * matchExpr();
			ASTNode * matchTerm();
			ASTNode * matchFactor();
			ASTNode * matchValue();

		private:
			// Private Methods;
			token getToken() const;
			token getToken(const size_t &d) const;
			// Methods for EXPR Parser
			std::pair<bool, parse::optype> parseSecondOp();
			std::pair<bool, parse::optype> parseCondJoinOp();
			// Member variables
			size_t pos_ = 0;
			Lexer *lex_ = 0;
	};
}
