//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Lexer.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the Fox Lexer.
//----------------------------------------------------------------------------//

#pragma once

#include "Token.hpp"
#include "Fox/Common/SourceLoc.hpp"
#include "Fox/Common/string_view.hpp"

namespace fox {
  class DiagnosticEngine;
  class SourceManager;

  // Lexer
  //    The Fox Lexer. An instance of the lexer is tied to a SourceFile 
  //    (FileID). The lexing process can be initiated by calling lex(), and
  //    the resulting tokens will be found in the token vector returned by
  //    getTokens()
  class Lexer  {
    public:
      // Constructor for the Lexer.
      Lexer(SourceManager& srcMgr, DiagnosticEngine& diags, FileID file);

      // Lex the full file
      void lex();
  
      // Return the tokens
      TokenVector& getTokens();

      // Returns the number of tokens in the vector
      std::size_t numTokens() const;  

      // The DiagnosticEngine instance tied to this Lexer
      DiagnosticEngine& diagEngine;
      // The SourceManager instance tied to this Lexer
      SourceManager& sourceMgr;
      // The FileID of the file being lexed
      const FileID theFile;

      // The Lexer should be movable but not copyable.
      Lexer(const Lexer&) = delete;
      Lexer& operator=(const Lexer&) = delete;
      Lexer(Lexer&&) = default;
      Lexer& operator=(Lexer&&) = default;
    private:
      // Pushes the current token with the kind "kind"
      void pushTok(TokenKind kind);

      // Pushes the current token character as a token with the kind "kind" 
      // (calls resetToken() + pushToken())
      void beginAndPushToken(TokenKind kind);

      // Calls advance(), then pushes the current token with the kind 'kind'
      void advanceAndPushTok(TokenKind kind);

      // Returns true if we reached EOF.
      bool isEOF() const;

      // Begins a new token
      void resetToken();

      // Entry point of the lexing process
      void lexImpl();

      // Lex an identifier or keyword
      void lexIdentifierOrKeyword();
      // Lex an int or double literal
      void lexIntOrDoubleConstant();
      // Lex any number of char items until we find a char that can't be
      // a char item, or if we find the (unescaped) delimiter or EOF.
      // Returns true if the delimiter was found
      bool lexCharItems(FoxChar delimiter);
      // Lex a piece of text delimited by single quotes '
      void lexSingleQuoteText();
      // Lex a piece of text delimited by double quotes "
      void lexDoubleQuoteText();
      // Lex an integer literal
      void lexIntConstant();

      // Handles a single-line comment
      void skipLineComment();
      // Handles a multi-line comment
      void skipBlockComment();

      // Returns false if the character cannot be considered a <char_item>
      bool canBeCharItem(FoxChar ch) const;

      // Returns true if 'ch' is a valid identifier head.
      bool isValidIdentifierHead(FoxChar ch) const;
      // Returns true if 'ch' is a valid identifier character.
      bool isValidIdentifierChar(FoxChar ch) const;

      // Returns the current character being considered
      FoxChar getCurChar() const;
      // Peeks the next character that will be considered
      FoxChar peekNextChar() const;

      // Advances to the next codepoint in the input.
      // Returns false if we reached EOF.
      bool advance();

      SourceLoc getLocOfPtr(const char* ptr) const;
      SourceLoc getCurPtrLoc() const;
      SourceLoc getCurtokBegLoc() const;
      SourceRange getCurtokRange() const;
      string_view getCurtokStringView() const;

      const char* fileBeg_  = nullptr;
      const char* fileEnd_ = nullptr;
      // The pointer to the first byte of the token
      const char* tokBegPtr_  = nullptr;
      // The pointer to the current character being considered.
      // (pointer to the first byte in the UTF8 codepoint)
      const char* curPtr_  = nullptr;

      TokenVector tokens_;
  };
}
