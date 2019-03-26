//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Token.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the Token class and Token kinds enums.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/AST/Identifier.hpp"
#include "Fox/Common/FoxTypes.hpp"
#include "Fox/Common/SourceLoc.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"
#include <cstddef>
#include <iosfwd>

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
    S_LESS_THAN,    // <
    S_GREATER_THAN, // >
    S_PERCENT,      // %

    // BRACKETS
    S_CURLY_OPEN,   // {
    S_CURLY_CLOSE,  // }
    S_SQ_OPEN,      // [
    S_SQ_CLOSE,     // ]
    S_ROUND_OPEN,   // (
    S_ROUND_CLOSE,  // )

    // PUNCTUATION
    S_EXCL_MARK,    // '!'
    S_SEMICOLON,    // ;
    S_COLON,        // :
    S_DOT,          // .
    S_COMMA,        // ,

    // Operators
    // FIXME: This is a quick hack to get the new lexer working.
    // Remove this from here later.
    S_OP_EXP,       // **
    S_OP_EQ,        // ==
    S_OP_INEQ,      // !=
    S_OP_LTEQ,      // <=
    S_OP_GTEQ,      // >=
    S_OP_LAND,      // &&
    S_OP_LOR,       // ||
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
  };

  struct Token  {
    public:
      enum class Kind : std::uint8_t {
        Invalid,
        Keyword,
        Sign,
        Identifier,
        // Literals
        IntLiteral,
        DoubleLiteral,
        BoolLiteral,
        DoubleQuoteTextLiteral,
        SingleQuoteTextLiteral
      };

      // Creates an invalid token
      Token() = default;
      // Creates a normal token
      Token(Kind kind, string_view str, SourceRange range);
      Token(SignType sign, string_view str, SourceRange range);
      Token(KeywordType kw, string_view str, SourceRange range);

      bool isValid() const;
      explicit operator bool() const;

      bool isAnyLiteral() const;
      bool isIdentifier() const;
      bool isSign() const;
      bool isKeyword() const;
      bool isBoolLiteral() const;
      bool isDoubleLiteral() const;
      bool isIntLiteral() const;
      bool isSingleQuoteTextLiteral() const;
      bool isDoubleQuoteTextLiteral() const;

      bool is(KeywordType ty);
      bool is(SignType ty);

      // For Keyword tokens, return the Keyword type.
      KeywordType getKeywordType() const;

      // For Sign tokens, return the Sign type.
      SignType getSignType() const;

      void dump(std::ostream& out) const;

      const SourceRange range;
      const string_view str;
      const Kind kind = Kind::Invalid;

    private:
      union {
        KeywordType kwType_;
        SignType signType_;
      };
  };

  // A Vector of Tokens.
  using TokenVector = SmallVector<Token, 4>;
}
