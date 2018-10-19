////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Token.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the declaration of various Token-related structures and enumerations,
// including the Token struct and LiteralInfo struct.
//
// TODO: This is spaghetti code I wrote very early in the project, rewrite this using a proper
// class hierarchy, which makes a lot more sense.
////------------------------------------------------------////

#pragma once

#include "Fox/Common/Typedefs.hpp"
#include "Fox/Common/Source.hpp"
#include "mpark/variant.hpp"
#include <vector>
#include <memory>
#include <map>

namespace fox
{
	class ASTContext;
	class Identifier;
	class DiagnosticEngine;

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
			LiteralInfo(FoxBool bval);
			LiteralInfo(const FoxString& sval);
			LiteralInfo(FoxFloat fval);
			LiteralInfo(FoxInt ival);
			LiteralInfo(FoxChar cval);

			bool isNull() const;
			LiteralType getType() const;
			
			bool isBool() const;
			bool isString() const;
			bool isFloat() const;
			bool isInt() const;
			bool isChar() const;

			FoxString getAsString() const;

			template<typename Ty>
			inline bool is() const
			{
				return mpark::holds_alternative<Ty>(value_);
			}

			template<typename Ty>
			inline Ty get() const
			{
				if (mpark::holds_alternative<Ty>(value_))
					return mpark::get<Ty>(value_);
				return Ty();
			}
		private:
			mpark::variant<
				mpark::monostate, FoxBool, FoxString, FoxFloat, FoxInt, FoxChar
			> value_;
	};

	enum class SignType : std::uint8_t
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

	enum class KeywordType : std::uint8_t
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

	enum class TokenType : std::uint8_t
	{
		UNKNOWN,
		LITERAL,
		SIGN,
		KEYWORD,
		IDENTIFIER, 
		COMMENT
	};

	struct Token 
	{
		public:

			Token() = default;
			Token(const Token& cpy);

			// Constructor to use to let the Token identify itself
			Token(DiagnosticEngine& diag, ASTContext &astctxt, 
				const std::string& tokstr, SourceRange range = SourceRange());

			std::string showFormattedTokenData() const;

			bool isValid() const;
			explicit operator bool() const;

			bool isLiteral() const;
			bool isIdentifier() const;
			bool isSign() const;
			bool isKeyword() const;

			bool is(KeywordType ty);
			bool is(SignType ty);
			bool is(LiteralType ty);

			KeywordType getKeywordType() const;
			SignType getSignType() const;
			LiteralType getLiteralType() const;
			LiteralInfo getLiteralInfo() const;

			std::string getIdentifierString() const;
			Identifier* getIdentifierInfo();

			std::string getAsString() const;
			std::string getTokenTypeFriendlyName() const;

			SourceRange getRange() const;

		private:
			/* Member variables */
			// Note: LiteralInfo is quite heavy, so it's dynamically allocated to save space, since
			// most token won't need it. Same goes for CommentData.
			const SourceRange range_;
			mpark::variant<mpark::monostate, KeywordType, SignType, Identifier*> tokenData_;
			std::unique_ptr<LiteralInfo> literalData_ = nullptr;

			/* Identification functions */
			void identify(DiagnosticEngine& diags, ASTContext& astctxt, const std::string& str);
			bool idKeyword(const std::string& str);
			bool idSign(const std::string& str);
			bool idLiteral(DiagnosticEngine& diags, const std::string& str);
			bool idIdentifier(DiagnosticEngine& diags, ASTContext& astctxt,const std::string& str);
			bool validateIdentifier(DiagnosticEngine& diags, const std::string& str) const;
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

		const std::map<FoxChar, SignType> kSign_dict =
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
	using TokenVector = std::vector<Token>;
}
