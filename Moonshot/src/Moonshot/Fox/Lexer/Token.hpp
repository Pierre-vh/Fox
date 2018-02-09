////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Token.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains some Token enumeration declarations	
// And the Declaration of the Token and text_pos structs.	
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


	enum class tokenCat
	{
		TT_ENUM_DEFAULT,	// Default value

		TT_IDENTIFIER,		// User defined identifiers.	(Foo, bar ...)
		TT_KEYWORD,			// Reserved keywords			(func,let ...)
		TT_SIGN,			// + - ; ( ] ...
		TT_LITERAL			// value ("hello", 3.14, 'c', -1, ...)
	};

	enum class literal
	{
		LIT_ENUMDEFAULT,		// Default value

		LIT_CHAR,
		LIT_INTEGER,
		LIT_FLOAT,
		LIT_BOOL,
		LIT_STRING
	};

	enum class sign
	{
		S_ENUM_DEFAULT,			// Default value
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
		S_CARET,				// ^
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

	enum class keyword
	{
		KW_ENUM_DEFAULT,		// Default value
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
	
	// Dictionary used to identify keywords.
	const std::map<std::string, keyword> kWords_dict =
	{
		// TYPES
		{ "int"		, keyword::T_INT		},
		{ "float"	, keyword::T_FLOAT	},
		{ "bool"	, keyword::T_BOOL	},
		{ "string"	, keyword::T_STRING	},
		{ "char"	, keyword::T_CHAR	},

		{ "const"	, keyword::T_CONST },
		// TYPE CONVERSION
		{ "as"		, keyword::TC_AS		},
		// DECLARATIONS
		{ "let"		, keyword::D_LET		},
		{ "func"	, keyword::D_FUNC	},
		{ "if"		, keyword::D_IF		},
		{ "else"	, keyword::D_ELSE	},
		{ "while"	, keyword::D_WHILE	},
		{ "return"	, keyword::D_RETURN	},
		// PACKAGES
		{ "import"	, keyword::P_IMPORT	},
		{ "using"	, keyword::P_USING	}

	};

	const std::map<CharType, sign> kSign_dict =
	{
		//signs
		{ '='	, sign::S_EQUAL			},
		{ '+'	, sign::S_PLUS			},
		{ '-'	, sign::S_MINUS			},
		{ '*'	, sign::S_ASTERISK		},
		{ '/'	, sign::S_SLASH			},
		{ '|'	, sign::S_VBAR			},
		{ '&'	, sign::S_AND			},
		{ '<'	, sign::S_LESS_THAN		},
		{ '>'	, sign::S_GREATER_THAN	},
		{ '#'	, sign::S_HASH			},
		{ '~'	, sign::S_TILDE			},
		{ '^'	, sign::S_CARET			},
		{ '%'	, sign::S_PERCENT		},
		// bracket
		{ '{'	, sign::B_CURLY_OPEN	},
		{ '}'	, sign::B_CURLY_CLOSE	},
		{ '['	, sign::B_SQ_OPEN		},
		{ ']'	, sign::B_SQ_CLOSE		},
		{ '('	, sign::B_ROUND_OPEN	},
		{ ')'	, sign::B_ROUND_CLOSE	},
		// punctuation
		{ ';'	, sign::P_SEMICOLON		},
		{ ':'	, sign::P_COLON			},
		{ '!'	, sign::P_EXCL_MARK		},
		{ '?'	, sign::P_INTER_MARK	},
		{ '.'	, sign::P_DOT			},
		{ ','	, sign::P_COMMA			}
	};

	struct text_pos	// a structure to hold the position of a Token in the input, and interact with it.
	{
		text_pos();
		text_pos(const int &l, const int &col);
		void newLine();
		void forward();
		std::string asText() const;

		int line = 1;
		int column = 0;
	};
	struct Token // the Token struct. The lexer outputs a std::vector<Token>. Tokens are recognized bits of the original input : keywords,id,values,etc.
	{
		public:
			Token(Context & c);
			Token(Context & c,std::string data, const text_pos &tpos = text_pos(0,0));
			tokenCat type =		tokenCat::TT_ENUM_DEFAULT;
			keyword kw_type =	keyword::KW_ENUM_DEFAULT;
			literal lit_type =	literal::LIT_ENUMDEFAULT;
			sign sign_type =	sign::S_ENUM_DEFAULT;

			FVal lit_val;

			std::string str;
			text_pos pos;

			std::string showFormattedTokenData() const;

			bool isValid() const;

		private:
			Context& context_;

			bool empty_ = false;
			void idToken();					// will id the tolen and call the specific evaluations functions if needed.
			bool specific_idKeyword();
			bool specific_idSign();
			bool specific_idValue();				// is a value (raw const)
										// this class will also put the value in the variant, with the exception of strings (it'll just trim the quotes and put it back in str)
	};
	// TokenVector typedef
	typedef std::vector<Token> TokenVector;
}
