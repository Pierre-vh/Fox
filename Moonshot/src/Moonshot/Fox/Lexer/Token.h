/************************************************************
Author : Pierre van Houtryve
Contact :
e-mail : pierre.vanhoutryve@gmail.com

Used by the Lexer class.
Description : Defines a structure and an enumeration to hold the token's information. 


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

#pragma once

#include <map>		// std::map 

namespace Moonshot::lex
{
	enum tokentype
	{
		TT_ENUM_DEFAULT,	// Default value

		TT_IDENTIFIER,		// User defined identifiers.	(Foo, bar ...)
		TT_KEYWORD,			// Reserved keywords			(func,let ...)
		TT_SIGN,			// + - ; ( ] ...
		TT_VALUE			// value ("hello", 3.14, 'c', -1, ...)
	};

	enum values
	{
		VAL_ENUM_DEFAULT,		// Default value

		VAL_CHAR,
		VAL_INTEGER,
		VAL_FLOAT,
		VAL_BOOL,
		VAL_STRING
	};

	enum signs
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
			S_EXP,				// ^
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
			P_INTER_MARK		// ?
	};

	enum keywords
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
	const std::map<std::string,keywords> kw_dict = 
	{
		// TYPES
		{ "int"		, T_INT		},
		{ "float"	, T_FLOAT	},
		{ "bool"	, T_BOOL	},
		{ "string"	, T_STRING	},
		{ "char"	, T_CHAR	},
		{ "const"	, T_CONST },
		// TYPE CONVERSION
		{ "AS"		, TC_AS		},
		// DECLARATIONS
		{ "let"		, D_LET		},
		{ "func"	, D_FUNC	},
		{ "if"		, D_IF		},
		{ "else"	, D_ELSE	},
		{ "while"	, D_WHILE	},
		{ "return"	, D_RETURN	},
		// PACKAGES
		{ "import"	, P_IMPORT	},
		{ "using"	, P_USING	}

	};

	const std::map<char,signs> sign_dict = 
	{
		//signs
		{ '='	, S_EQUAL			},
		{ '+'	, S_PLUS			},
		{ '-'	, S_MINUS			},
		{ '*'	, S_ASTERISK		},
		{ '/'	, S_SLASH			},
		{ '|'	, S_VBAR			},
		{ '&'	, S_AND				},
		{ '<'	, S_LESS_THAN		},
		{ '>'	, S_GREATER_THAN	},
		{ '#'	, S_HASH			},
		{ '~'	, S_TILDE			},
		{ '^'	, S_EXP				},
		{ '%'	, S_PERCENT			},
		// brackets
		{ '{'	, B_CURLY_OPEN		},
		{ '}'	, B_CURLY_CLOSE		},
		{ '['	, B_SQ_OPEN			},
		{ ']'	, B_SQ_CLOSE		},
		{ '('	, B_ROUND_OPEN		},
		{ ')'	, B_ROUND_CLOSE		},
		// punctuation
		{ ';'	, P_SEMICOLON		},
		{ ':'	, P_COLON			},
		{ '!'	, P_EXCL_MARK		},
		{ '?'	, P_INTER_MARK		},

	};

	struct token
	{
		tokentype type = TT_ENUM_DEFAULT;
		keywords kw_type = KW_ENUM_DEFAULT;
		values val_type = VAL_ENUM_DEFAULT;
		signs sign_type = S_ENUM_DEFAULT,
	};
}
