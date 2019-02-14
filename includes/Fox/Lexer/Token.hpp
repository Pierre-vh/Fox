//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Token.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the declaration of various Token-related structures and enumerations,
// including the Token struct and LiteralInfo struct.
//
// TODO: This is spaghetti code I wrote very early in the project, rewrite this using a proper
// class hierarchy, which makes a lot more sense.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/AST/Identifier.hpp"
#include "Fox/Common/Typedefs.hpp"
#include "Fox/Common/SourceLoc.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"
#include <memory>
#include <map>

namespace fox {
  class ASTContext;
  class DiagnosticEngine;

  enum class SignType : std::uint8_t {
    DEFAULT,      // Default value
    // Signs
    S_EQUAL,        // =
    S_PLUS,         // +
    S_MINUS,        // -
    S_ASTERISK,     // *
    S_SLASH,        // /
    S_VBAR,         // |
    S_AMPERSAND,    // &
    S_LESS_THAN,    // <
    S_GREATER_THAN, // >
    S_HASH,         // #
    S_TILDE,        // ~
    S_CARET,        // ^
    S_PERCENT,      // %

    // BRACKETS
    S_CURLY_OPEN,   // {
    S_CURLY_CLOSE,  // }
    S_SQ_OPEN,      // [
    S_SQ_CLOSE,     // ]
    S_ROUND_OPEN,   // (
    S_ROUND_CLOSE,  // )
    // PUNCTUATION
    S_SEMICOLON,    // ;
    S_COLON,        // :
    S_EXCL_MARK,    // !
    S_INTER_MARK,   // ?
    S_DOT,          // .
    S_COMMA         // ,
  };

  enum class KeywordType : std::uint8_t {
    DEFAULT,  // Default value
    // TYPES
    KW_INT,     // "int"
    KW_DOUBLE,  // "double"
    KW_BOOL,    // "bool"
    KW_STRING,  // "string"
    KW_CHAR,    // "char"
    // PARAMETER MODIFIERS
    KW_MUT,     // "mut"
    // TYPE CONVERSION
    KW_AS,      // "as"
    // DECLARATION / STATEMENT
    KW_LET,     // "let"
    KW_VAR,     // "var"
    KW_FUNC,    // "func"
    KW_IF,      // "if"
    KW_ELSE,    // "else"
    KW_WHILE,   // "while"
    KW_RETURN,  // "return"
    // PACKAGE
    KW_IMPORT,  // "import"
    KW_USING    // "using"
  };

  struct Token  {
    public:
      // Creates an invalid token
      Token() = default;

      // Constructor to use to let the Token identify itself
      Token(ASTContext &astctxt, const std::string& tokstr, 
        SourceRange range = SourceRange());

      std::string showFormattedTokenData() const;

      bool isValid() const;
      explicit operator bool() const;

      // General categories
      bool isLiteral() const;
      bool isIdentifier() const;
      bool isSign() const;
      bool isKeyword() const;
      // Literals
      bool isStringLiteral() const;
      bool isBoolLiteral() const;
      bool isDoubleLiteral() const;
      bool isIntLiteral() const;
      bool isCharLiteral() const;

      bool is(KeywordType ty);
      bool is(SignType ty);

      // For Keyword tokens, return the Keyword type.
			//  Asserts that this token is an Keyword token.
      KeywordType getKeywordType() const;

      // For Sign tokens, return the Sign type.
			//  Asserts that this token is an Sign token.
      SignType getSignType() const;

      // For Identifier tokens, return the Identifier object.
			//  Asserts that this token is an Identifier token.
      Identifier getIdentifier() const;

      // For BoolLiteral tokens, return the Boolean value.
			//  Asserts that this token is an BoolLiteral token.
      bool getBoolValue() const;

      // For StringLiteral tokens, return the String value.
			//  Asserts that this token is an StringLiteral token.
      // NOTE: This returns a string_view which is a view of a string
      //       stored in the ASTContext. This means that the string_view
      //       returned can be used to create ASTNodes without any issues.
      string_view getStringValue() const;

      // For CharLiteral tokens, return the Char value.
			//  Asserts that this token is an CharLiteral token.
      FoxChar getCharValue() const;

      // For IntLiteral tokens, return the Int value.
			//  Asserts that this token is an IntLiteral token.
      FoxInt getIntValue() const;

      // For DoubleLiteral tokens, return the Double value.
			//  Asserts that this token is an DoubleLiteral token.
      FoxDouble getDoubleValue() const;

      std::string getAsString() const;
      std::string getTokenTypeFriendlyName() const;

      SourceRange getRange() const;

    private:
      enum class Kind : unsigned char {
        Invalid,
        Keyword,
        Sign,
        Identifier,
        // Literals
        IntLiteral,
        DoubleLiteral,
        BoolLiteral,
        StringLiteral,
        CharLiteral,
      };

      const SourceRange range_;

      struct Data {
        Data() : kind(Kind::Invalid) {}
        Kind kind;
        union {
          KeywordType keyword;
          SignType sign;
          Identifier identifier;
          // Literals
          FoxInt intLiteral;
          FoxChar charLiteral;
          bool boolLiteral;
          string_view stringLiteral;
          FoxDouble doubleLiteral;
        };

        #define SETTER(KIND, TYPE, MEMBER)\
          void set##KIND(TYPE value) { \
            kind = Kind::KIND;\
            MEMBER = value; \
          }

        SETTER(Keyword, KeywordType, keyword)
        SETTER(Sign, SignType, sign)
        SETTER(Identifier, Identifier, identifier)
        SETTER(IntLiteral, FoxInt, intLiteral)
        SETTER(DoubleLiteral, double, doubleLiteral)
        SETTER(BoolLiteral, bool, boolLiteral)
        SETTER(StringLiteral, string_view, stringLiteral)
        SETTER(CharLiteral, FoxChar, charLiteral)

        #undef SETTER
      } data_;

      /* Identification functions */
      void identify(ASTContext& astctxt, const std::string& str);
      bool idKeyword(const std::string& str);
      bool idSign(const std::string& str);
      bool idLiteral(ASTContext& ctxt, DiagnosticEngine& diags, 
                    const std::string& str);
      bool idIdentifier(ASTContext& astctxt, 
                        const std::string& str);
      bool validateIdentifier(string_view id) const;
      // Helper for idIdentifier 
      bool hasAtLeastOneLetter(const std::string &str) const; // Checks if str_ has at least one upper/lower case letter.
  };

  namespace dicts {
    const std::map<std::string, KeywordType> kKeywords_dict = {
      // TYPES 
      { "int",    KeywordType::KW_INT },
      { "double", KeywordType::KW_DOUBLE },
      { "bool",   KeywordType::KW_BOOL },
      { "string", KeywordType::KW_STRING },
      { "char",   KeywordType::KW_CHAR },
      // PARAM MODIFIERS 
      { "mut",    KeywordType::KW_MUT },
      // TYPE CONVERSION
      { "as",     KeywordType::KW_AS },
      // DECLARATIONS
      { "let",    KeywordType::KW_LET },
      { "var",    KeywordType::KW_VAR },
      { "func",   KeywordType::KW_FUNC },
      // Statements 
      { "if",     KeywordType::KW_IF },
      { "else",   KeywordType::KW_ELSE }, 
      { "while",  KeywordType::KW_WHILE },
      // return
      { "return", KeywordType::KW_RETURN },
      // import
      { "import", KeywordType::KW_IMPORT },
      { "using",  KeywordType::KW_USING }
    };

    const std::map<FoxChar, SignType> kSign_dict = {
      // signs
      { '=', SignType::S_EQUAL },
      { '+', SignType::S_PLUS },
      { '-', SignType::S_MINUS },
      { '*', SignType::S_ASTERISK },
      { '/', SignType::S_SLASH },
      { '|', SignType::S_VBAR },
      { '&', SignType::S_AMPERSAND },
      { '<', SignType::S_LESS_THAN },
      { '>', SignType::S_GREATER_THAN },
      { '#', SignType::S_HASH }, 
      { '~', SignType::S_TILDE },
      { '^', SignType::S_CARET },
      { '%', SignType::S_PERCENT },
      // brackets
      { '{', SignType::S_CURLY_OPEN },
      { '}', SignType::S_CURLY_CLOSE },
      { '[', SignType::S_SQ_OPEN },
      { ']', SignType::S_SQ_CLOSE },
      { '(', SignType::S_ROUND_OPEN },
      { ')', SignType::S_ROUND_CLOSE },
      // punctuation 
      { ';', SignType::S_SEMICOLON },
      { ':', SignType::S_COLON },
      { '!', SignType::S_EXCL_MARK },
      { '?', SignType::S_INTER_MARK },
      { '.', SignType::S_DOT },
      { ',', SignType::S_COMMA }
    };
  }

  // A Vector of Tokens.
  using TokenVector = SmallVector<Token, 4>;
}
