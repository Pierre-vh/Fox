#pragma once

#include <map>		// std::map 
#include <variant>	// std::variant
#include <regex>	// std::regex, std::regex_match
#include <string>	// std::stoi
#include <sstream>	// std::stringstream (showFormattedToken())

namespace Moonshot
{
	namespace lex
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
			P_INTER_MARK,		// ?
			P_DOT,				// .
			P_COMMA				// ,
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
		// Regular expression used for identification 
		const std::regex kInt_regex("\\d+");
		const std::regex kFloat_regex("[0-9]*\\.?[0-9]+");
		const std::regex kId_regex("(([A-Z]|[a-z]|_)([A-Z]|[0-9]|[a-z]|_)?)+");	// if anyone has something better, tell me ! :)

		// Dictionary used to identify keywords.
		const std::map<std::string, lex::keywords> kWords_dict =
		{
			// TYPES
			{ "int"		, T_INT		},
			{ "float"	, T_FLOAT	},
			{ "bool"	, T_BOOL	},
			{ "string"	, T_STRING	},
			{ "char"	, T_CHAR	},

			{ "const"	, T_CONST },
			// TYPE CONVERSION
			{ "as"		, TC_AS		},
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

		const std::map<char, lex::signs> kSign_dict =
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
			{ '.'	, P_DOT				},
			{ ','	, P_COMMA			}	
		};
	}
	struct text_pos	// a structure to hold the position of a token in the input;
	{
		text_pos();
		text_pos(const int &l, const int &col);
		void newLine();
		void forward();
		std::string asText() const;

		int line = 1;
		int column = 0;
	};
	struct token
	{
		public:
			token();
			token(std::string data, const text_pos &tpos = text_pos(0,0));
			lex::tokentype type = lex::TT_ENUM_DEFAULT;
			lex::keywords kw_type = lex::KW_ENUM_DEFAULT;
			lex::values val_type = lex::VAL_ENUM_DEFAULT;
			lex::signs sign_type = lex::S_ENUM_DEFAULT;

			std::string str;
			text_pos pos;
			std::variant<bool, int, float>	vals;

			std::string showFormattedTokenData() const;

			bool isValid() const;

		private:
			bool empty_ = false;
			void selfId();
			bool idKeyword();
			bool idSign();
			bool idValue();				// is a value (raw const)
										// this class will also put the value in the variant, with the exception of strings (it'll just trim the quotes and put it back in str)
	};
}
