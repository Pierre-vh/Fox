////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Token.h											
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
#include <variant>	// std::variant
#include <regex>	// std::regex, std::regex_match
#include <string>	// std::stoi
#include <sstream>	// std::stringstream (showFormattedToken())
#include <vector>

#include "../../Common/Utils/Utils.h"
#include "../../Common/Context//Context.h"
#include "../../Common/Exceptions/Exceptions.h"

namespace Moonshot
{

	enum class tokenType
	{
		TT_ENUM_DEFAULT,	// Default value

		TT_IDENTIFIER,		// User defined identifiers.	(Foo, bar ...)
		TT_KEYWORD,			// Reserved keywords			(func,let ...)
		TT_SIGN,			// + - ; ( ] ...
		TT_LITERAL			// value ("hello", 3.14, 'c', -1, ...)
	};

	enum class literalType
	{
		LIT_ENUMDEFAULT,		// Default value

		LIT_CHAR,
		LIT_INTEGER,
		LIT_FLOAT,
		LIT_BOOL,
		LIT_STRING
	};

	enum class signType
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
		P_INTER_MARK,		// ?
		P_DOT,				// .
		P_COMMA				// ,
	};

	enum class keywordType
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
	// Regular expression used for identification 
	const std::regex kInt_regex("\\d+");
	const std::regex kFloat_regex("[0-9]*\\.?[0-9]+");
	const std::regex kId_regex("(([A-Z]|[a-z]|_)([A-Z]|[0-9]|[a-z]|_)?)+");	// if anyone has something better, tell me ! :)

	// Dictionary used to identify keywords.
	const std::map<std::string, keywordType> kWords_dict =
	{
		// TYPES
		{ "int"		, keywordType::T_INT		},
		{ "float"	, keywordType::T_FLOAT	},
		{ "bool"	, keywordType::T_BOOL	},
		{ "string"	, keywordType::T_STRING	},
		{ "char"	, keywordType::T_CHAR	},

		{ "const"	, keywordType::T_CONST },
		// TYPE CONVERSION
		{ "as"		, keywordType::TC_AS		},
		// DECLARATIONS
		{ "let"		, keywordType::D_LET		},
		{ "func"	, keywordType::D_FUNC	},
		{ "if"		, keywordType::D_IF		},
		{ "else"	, keywordType::D_ELSE	},
		{ "while"	, keywordType::D_WHILE	},
		{ "return"	, keywordType::D_RETURN	},
		// PACKAGES
		{ "import"	, keywordType::P_IMPORT	},
		{ "using"	, keywordType::P_USING	}

	};

	const std::map<char, signType> kSign_dict =
	{
		//signs
		{ '='	, signType::S_EQUAL			},
		{ '+'	, signType::S_PLUS			},
		{ '-'	, signType::S_MINUS			},
		{ '*'	, signType::S_ASTERISK		},
		{ '/'	, signType::S_SLASH			},
		{ '|'	, signType::S_VBAR			},
		{ '&'	, signType::S_AND			},
		{ '<'	, signType::S_LESS_THAN		},
		{ '>'	, signType::S_GREATER_THAN	},
		{ '#'	, signType::S_HASH			},
		{ '~'	, signType::S_TILDE			},
		{ '^'	, signType::S_EXP			},
		{ '%'	, signType::S_PERCENT		},
		// bracket
		{ '{'	, signType::B_CURLY_OPEN	},
		{ '}'	, signType::B_CURLY_CLOSE	},
		{ '['	, signType::B_SQ_OPEN		},
		{ ']'	, signType::B_SQ_CLOSE		},
		{ '('	, signType::B_ROUND_OPEN	},
		{ ')'	, signType::B_ROUND_CLOSE	},
		// punctuation
		{ ';'	, signType::P_SEMICOLON		},
		{ ':'	, signType::P_COLON			},
		{ '!'	, signType::P_EXCL_MARK		},
		{ '?'	, signType::P_INTER_MARK	},
		{ '.'	, signType::P_DOT			},
		{ ','	, signType::P_COMMA			}
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
			tokenType type =		tokenType::TT_ENUM_DEFAULT;
			keywordType kw_type =	keywordType::KW_ENUM_DEFAULT;
			literalType val_type =	literalType::LIT_ENUMDEFAULT;
			signType sign_type =	signType::S_ENUM_DEFAULT;

			std::variant<int, bool, std::string, char, float> vals;

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
