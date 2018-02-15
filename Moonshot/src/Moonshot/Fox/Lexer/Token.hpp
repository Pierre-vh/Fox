////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Token.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains some Token enumeration declarations	
// And the Declaration of the Token and TextPosition structs.	
//															
// This file also declares some const variables	holding regexes, 
// and maps for getting a "friendly name" of enum values.									
////------------------------------------------------------////

#pragma once

#include <map>		// std::map 
#include "Moonshot/Common/Types/Types.hpp"
#include <vector>

namespace Moonshot
{
	class Context;

	struct TextPosition	// a structure to hold the position of a Token in the input, and interact with it.
	{
		TextPosition();
		TextPosition(const int &l, const int &col);
		void newLine();
		void forward();
		std::string asText() const;

		unsigned int line = 1;
		unsigned int column = 1;
	};
	struct Token // the Token struct. The lexer outputs a std::vector<Token>. Tokens are recognized bits of the original input : keywords,id,values,etc.
	{
		public:
			enum class category : char
			{
				DEFAULT,	// Default value
				IDENTIFIER,		// User defined identifiers.	(Foo, bar ...)
				KEYWORD,			// Reserved keywords			(func,let ...)
				SIGN,			// + - ; ( ] ...
				LITERAL			// value ("hello", 3.14, 'c', -1, ...)
			};
			enum class literal : char
			{
				DEFAULT,		// Default value
				LIT_CHAR,
				LIT_INTEGER,
				LIT_FLOAT,
				LIT_BOOL,
				LIT_STRING
			};
			enum class sign : char
			{
				DEFAULT,			// Default value
				// Signs
				S_EQUAL,			// =
				S_PLUS,				// +
				S_MINUS,			// -
				S_ASTERISK,			// *
				S_SLASH,			// /
				S_VBAR,				// |
				S_AND,				// &
				S_LESS_THAN,		// <
				S_GREATER_THAN,		// >
				S_HASH,				// #
				S_TILDE,			// ~
				S_CARET,			// ^
				S_PERCENT,			// %

				// BRACKETS
				B_CURLY_OPEN,		// {
				B_CURLY_CLOSE,		// }
				B_SQ_OPEN,			// [
				B_SQ_CLOSE,			// ]
				B_ROUND_OPEN,		// (
				B_ROUND_CLOSE,		// )
				// PUNCTUATION
				P_SEMICOLON,		// ;
				P_COLON,			// :
				P_EXCL_MARK,		// !
				P_INTER_MARK,		// ?
				P_DOT,				// .
				P_COMMA				// ,
			};
			enum class keyword : char
			{
				DEFAULT,		// Default value
				// TYPES
				T_INT,				// "int"
				T_FLOAT,			// "float"
				T_BOOL,				// "bool"
				T_STRING,			// "string"
				T_CHAR,				// "char"
				T_CONST,			// "const"
				// TYPE CONVERSION
				TC_AS,				// "as"
				// DECLARATION
				D_LET,				// "let"
				D_FUNC,				// "func"
				D_IF,				// "if"
				D_ELSE,				// "else"
				D_WHILE,			// "while"
				D_RETURN,			// "return"
				// PACKAGE
				P_IMPORT,			// "import"
				P_USING				// "using"
			};

			Token(Context & c);
			Token(Context & c,std::string data, const TextPosition &tpos = TextPosition(0,0));
			category type =		category::DEFAULT;
			keyword kw_type =	keyword::DEFAULT;
			literal lit_type =	literal::DEFAULT;
			sign sign_type =	sign::DEFAULT;

			FVal lit_val;

			std::string str;
			TextPosition pos;

			std::string showFormattedTokenData() const;

			bool isValid() const;

		private:
			Context& context_;

			void idToken();					// will id the tolen and call the specific evaluations functions if needed.
			bool specific_idKeyword();
			bool specific_idSign();
			bool specific_idLiteral();				// is a literal
	};

	namespace TokenDictionaries
	{
		// Dictionary used to identify keywords.
		const std::map<std::string, Token::keyword> kKeywords_dict =
		{
			// TYPES
			{ "int"		, Token::keyword::T_INT },
			{ "float"	, Token::keyword::T_FLOAT },
			{ "bool"	, Token::keyword::T_BOOL },
			{ "string"	, Token::keyword::T_STRING },
			{ "char"	, Token::keyword::T_CHAR },
			// specifier
			{ "const"	, Token::keyword::T_CONST },
			// TYPE CONVERSION
			{ "as"		, Token::keyword::TC_AS },
			// DECLARATIONS
			{ "let"		, Token::keyword::D_LET },
			{ "func"	, Token::keyword::D_FUNC },
			{ "fn"		, Token::keyword::D_FUNC },
			// Statements
			{ "if"		, Token::keyword::D_IF },
			{ "else"	, Token::keyword::D_ELSE },
			{ "while"	, Token::keyword::D_WHILE },
			// return
			{ "return"	, Token::keyword::D_RETURN },
			// import
			{ "import"	, Token::keyword::P_IMPORT },
			{ "using"	, Token::keyword::P_USING }
			// Before release, I might add more keywords that could be used later. e.g. : class, static, void, null,for,do,break,continue, etc.
		};

		const std::map<CharType, Token::sign> kSign_dict =
		{
			//signs
			{ '='	, Token::sign::S_EQUAL },
			{ '+'	, Token::sign::S_PLUS },
			{ '-'	, Token::sign::S_MINUS },
			{ '*'	, Token::sign::S_ASTERISK },
			{ '/'	, Token::sign::S_SLASH },
			{ '|'	, Token::sign::S_VBAR },
			{ '&'	, Token::sign::S_AND },
			{ '<'	, Token::sign::S_LESS_THAN },
			{ '>'	, Token::sign::S_GREATER_THAN },
			{ '#'	, Token::sign::S_HASH },
			{ '~'	, Token::sign::S_TILDE },
			{ '^'	, Token::sign::S_CARET },
			{ '%'	, Token::sign::S_PERCENT },
			// bracket
			{ '{'	, Token::sign::B_CURLY_OPEN },
			{ '}'	, Token::sign::B_CURLY_CLOSE },
			{ '['	, Token::sign::B_SQ_OPEN },
			{ ']'	, Token::sign::B_SQ_CLOSE },
			{ '('	, Token::sign::B_ROUND_OPEN },
			{ ')'	, Token::sign::B_ROUND_CLOSE },
			// punctuation
			{ ';'	, Token::sign::P_SEMICOLON },
			{ ':'	, Token::sign::P_COLON },
			{ '!'	, Token::sign::P_EXCL_MARK },
			{ '?'	, Token::sign::P_INTER_MARK },
			{ '.'	, Token::sign::P_DOT },
			{ ','	, Token::sign::P_COMMA }
		};
	}

	// TokenVector typedef
	typedef std::vector<Token> TokenVector;
}
