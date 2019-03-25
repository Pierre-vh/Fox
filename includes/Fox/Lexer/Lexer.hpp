//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Lexer.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file declares the lexer class, which performs lexical analysis.    
//----------------------------------------------------------------------------//

// Note: Most of the code here is complete spaghetti that I wrote very early
// in the project.
// It'll be completly rewritten in the future. Same for Token.hpp/.cpp

#pragma once

#include "Token.hpp"
#include "Fox/Common/StringManipulator.hpp"

namespace fox {
  class DiagnosticEngine;
  class SourceManager;
  class Lexer  {
    public:
      Lexer(ASTContext &astctxt);

      void lexFile(FileID file);
  
      TokenVector& getTokenVector();
      std::size_t resultSize() const;  
      FileID getCurrentFile() const;

      ASTContext& ctxt;
      DiagnosticEngine& diagEngine;
      SourceManager& srcMgr;

    private:
      // Pushes the current token with the kind 'kind'
      template<typename Kind>
      void pushTok(Kind kind) {
        assert((tokBegPtr_ != curPtr_) && "empty token");
        // Push the token
        tokens_.push_back(Token(kind, getCurtokStringView(), getCurtokRange()));
        // Reset the iterators
        tokBegPtr_ = curPtr_;
      }

      // Pushes a token of kind 'kind' consisting of a single codepoint
      // (calls beginToken() + pushToken())
      template<typename Kind>
      void beginAndPushToken(Kind kind) {
        beginToken();
        pushTok(kind);
      }

      // Calls advance(), then pushes the current token with the kind 'kind'
      template<typename Kind>
      void advanceAndPushTok(Kind kind) {
        advance();
        pushTok(kind);
      }

      // Begins a new token
      void beginToken();

      void lex();
      void lexIdentifierOrKeyword();
      void lexIntOrDoubleLiteral();
      void lexIntLiteral();

      // Handles a single-line comment
      void skipLineComment();
      // Handles a multi-line comment
      void skipBlockComment();

      // Returns true if 'ch' is a valid identifier head.
      bool isValidIdentifierHead(FoxChar ch) const;

      // Returns the current character being considered
      FoxChar getCurChar() const;
      // Peeks the next character that will be considered
      FoxChar peekNextChar() const;

      // Advances to the next codepoint in the input.
      bool advance();

      SourceLoc getCurPtrLoc() const;
      SourceLoc getCurtokBegLoc() const;
      SourceRange getCurtokRange() const;
      string_view getCurtokStringView() const;

      FileID fileID_;

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
