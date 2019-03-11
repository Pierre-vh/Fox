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
      enum class DFAState : std::uint8_t {
        S_BASE, // basestate
        S_STR,  // string literals
        S_LCOM,  // line comment
        S_MCOM,  // multiline comment
        S_WORDS,// basic (keywords,signs) state
        S_CHR  // char literals
      };

      void pushTok();
      void cycle();          
      void runFinalChecks();

      void runStateFunc();
      void dfa_goto(DFAState ns);
      void fn_S_BASE();  // base state
      void fn_S_STR();  // string literals
      void fn_S_LCOM();  // one line comment
      void fn_S_MCOM();  // Multiple line comments
      void fn_S_WORDS();  // "basic" state (push to tok until separator is met)
      void fn_S_CHR();  // Char literals

      // returns the current char and run updatePos 
      // (returns inputstr_[pos_] and do pos_+=1)
      FoxChar eatChar();
      // adds the current character to curtok
      void addToCurtok(FoxChar c);
      // is the current char a separator? (= a sign. see kSign_dict)
      bool isSep(FoxChar c) const;
      // Checks if C is \ AND if the state is adequate for it
      // to be considered as an escape char.
      bool isEscapeChar(FoxChar c) const;
      // Checks if the char is valid to be pushed. 
      // If it isn't and it should be ignored, returns true
      bool shouldIgnore(FoxChar c) const;

      SourceLoc getCurtokBegLoc() const;
      SourceLoc getCurtokEndLoc() const;
      SourceRange getCurtokRange() const;

      FileID fileID_;

      bool escapeFlag_ : 1;
      DFAState state_ = DFAState::S_BASE;    
      std::string curtok_;

      std::size_t curTokBegIdx_ = 0, curTokEndIdx_ = 0;

      TokenVector tokens_;
      StringManipulator strManip_;
  };
}
