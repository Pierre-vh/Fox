////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Token.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the declaration of various Token-related structures and enumerations,
// including the Token struct and LiteralInfo struct.
////------------------------------------------------------////

#pragma once

#include "Moonshot/Fox/Common/Typedefs.hpp"
#include "Moonshot/Fox/Common/SourceManager.hpp"
#include <vector>
#include <variant>
#include <memory>
#include <map>

namespace Moonshot
{
	class Context;
	class ASTContext;
	class IdentifierInfo;
	class SourceRange;

	enum class LiteralType : char
	{
		DEFAULT,
		Ty_Bool,
		Ty_Float,
		Ty_Char,
		Ty_String,
		Ty_Int
	};

	struct LiteralInfo
	{
		public:
			LiteralInfo() = default;
			LiteralInfo(const bool& bval);
			LiteralInfo(const std::string& sval);
			LiteralInfo(const FloatType& fval);
			LiteralInfo(const IntType& ival);
			LiteralInfo(const CharType& cval);

			bool isNull() const;
			operator bool() const;
			LiteralType getType() const;
			
			bool isBool() const;
			bool isString() const;
			bool isFloat() const;
			bool isInt() const;
			bool isChar() const;

			std::string getAsString() const;

			template<typename Ty>
			inline bool is() const
			{
				return std::holds_alternative<Ty>(val_);
			}

			template<typename Ty>
			inline Ty get() const
			{
				if (std::holds_alternative<Ty>(val_))
					return std::get<Ty>(val_);
				return Ty();
			}
		private:
			std::variant<std::monostate,bool,std::string,FloatType,IntType,CharType> val_;
	};
	
	enum class SignType : char
	{
		DEFAULT,			// Default value
		// Signs
		S_EQUAL,			// =
		S_PLUS,				// +
		S_MINUS,			// -
		S_ASTERISK,			// *
		S_SLASH,			// /
		S_VBAR,				// |
		S_AMPERSAND,		// &
		S_LESS_THAN,		// <
		S_GREATER_THAN,		// >
		S_HASH,				// #
		S_TILDE,			// ~
		S_CARET,			// ^
		S_PERCENT,			// %

		// BRACKETS
		S_CURLY_OPEN,		// {
		S_CURLY_CLOSE,		// }
		S_SQ_OPEN,			// [
		S_SQ_CLOSE,			// ]
		S_ROUND_OPEN,		// (
		S_ROUND_CLOSE,		// )
		// PUNCTUATION
		S_SEMICOLON,		// ;
		S_COLON,			// :
		S_EXCL_MARK,		// !
		S_INTER_MARK,		// ?
		S_DOT,				// .
		S_COMMA				// ,
	};

	enum class KeywordType : char
	{
		DEFAULT,		// Default value
		// TYPES
		KW_INT,				// "int"
		KW_FLOAT,			// "float"
		KW_BOOL,			// "bool"
		KW_STRING,			// "string"
		KW_CHAR,			// "char"
		// QUALIFIERS
		KW_CONST,			// "const"
		// TYPE CONVERSION
		KW_AS,				// "as"
		// DECLARATION / STATEMENT
		KW_LET,				// "let"
		KW_FUNC,			// "func"
		KW_IF,				// "if"
		KW_ELSE,			// "else"
		KW_WHILE,			// "while"
		KW_RETURN,			// "return"
		// PACKAGE
		KW_IMPORT,			// "import"
		KW_USING			// "using"
	};

	enum class TokenType : char
	{
		UNKNOWN,
		LITERAL,
		SIGN,
		KEYWORD,
		IDENTIFIER
	};

	struct Token 
	{
		public:
			Token() = default;
			Token(const Token& cpy);
			Token(Context &ctxt,ASTContext &astctxt,std::string tokstr,const SourceRange& range = SourceRange());

			std::string showFormattedTokenData() const;

			bool isValid() const;
			operator bool() const;

			bool isLiteral() const;
			bool isIdentifier() const;
			bool isSign() const;
			bool isKeyword() const;

			bool is(const KeywordType &ty);
			bool is(const SignType &ty);
			bool is(const LiteralType& ty);

			KeywordType getKeywordType() const;
			SignType getSignType() const;
			LiteralType getLiteralType() const;
			LiteralInfo getLiteralInfo() const;

			std::string getIdentifierString() const;
			IdentifierInfo* getIdentifierInfo();

			std::string getAsString() const;
			std::string getTokenTypeFriendlyName() const;

			SourceRange& sourceRange();
			const SourceRange& sourceRange() const;
		private:
			// Empty struct used to "mark" the variant when this token is a literal.
			struct Literal {};

			/* Member variables */
			std::variant<std::monostate, KeywordType, SignType, Literal, IdentifierInfo *> tokenInfo_;
			std::unique_ptr<LiteralInfo> litInfo_ = nullptr;
			SourceRange range_;

			/* Identification functions */
			void idToken(Context& ctxt,ASTContext& astctxt, const std::string& str);
			bool specific_idKeyword(const std::string& str);
			bool specific_idSign(const std::string& str);
			bool specific_idLiteral(Context& ctxt, const std::string& str);
			bool specific_idIdentifier(Context& ctxt,ASTContext& astctxt,const std::string& str);
			bool validateIdentifier(Context& ctxt,const std::string& str) const;
			// Helper for idIdentifier
			bool hasAtLeastOneLetter(const std::string &str) const; // Checks if str_ has at least one upper/lower case letter.
	
	};
	namespace Dictionaries
	{
		const std::map<std::string, KeywordType> kKeywords_dict =
		{
			// TYPES
			{ "int"		, KeywordType::KW_INT },
			{ "float"	, KeywordType::KW_FLOAT },
			{ "bool"	, KeywordType::KW_BOOL },
			{ "string"	, KeywordType::KW_STRING },
			{ "char"	, KeywordType::KW_CHAR },
			// specifier
			{ "const"	, KeywordType::KW_CONST },
			// TYPE CONVERSION
			{ "as"		, KeywordType::KW_AS },
			// DECLARATIONS
			{ "let"		, KeywordType::KW_LET },
			{ "func"	, KeywordType::KW_FUNC },
			// Statements
			{ "if"		, KeywordType::KW_IF },
			{ "else"	, KeywordType::KW_ELSE },
			{ "while"	, KeywordType::KW_WHILE },
			// return
			{ "return"	, KeywordType::KW_RETURN },
			// import
			{ "import"	, KeywordType::KW_IMPORT },
			{ "using"	, KeywordType::KW_USING }
		};

		const std::map<CharType, SignType> kSign_dict =
		{
			//signs
			{ '='	, SignType::S_EQUAL },
			{ '+'	, SignType::S_PLUS },
			{ '-'	, SignType::S_MINUS },
			{ '*'	, SignType::S_ASTERISK },
			{ '/'	, SignType::S_SLASH },
			{ '|'	, SignType::S_VBAR },
			{ '&'	, SignType::S_AMPERSAND },
			{ '<'	, SignType::S_LESS_THAN },
			{ '>'	, SignType::S_GREATER_THAN },
			{ '#'	, SignType::S_HASH },
			{ '~'	, SignType::S_TILDE },
			{ '^'	, SignType::S_CARET },
			{ '%'	, SignType::S_PERCENT },
			// bracket
			{ '{'	, SignType::S_CURLY_OPEN },
			{ '}'	, SignType::S_CURLY_CLOSE },
			{ '['	, SignType::S_SQ_OPEN },
			{ ']'	, SignType::S_SQ_CLOSE },
			{ '('	, SignType::S_ROUND_OPEN },
			{ ')'	, SignType::S_ROUND_CLOSE },
			// punctuation
			{ ';'	, SignType::S_SEMICOLON },
			{ ':'	, SignType::S_COLON },
			{ '!'	, SignType::S_EXCL_MARK },
			{ '?'	, SignType::S_INTER_MARK },
			{ '.'	, SignType::S_DOT },
			{ ','	, SignType::S_COMMA }
		};
	}
	// TokenVector typedef
	typedef std::vector<Token> TokenVector;
}
