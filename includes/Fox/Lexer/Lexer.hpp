//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Lexer.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file declares the lexer class, which performs lexical analysis.    
//----------------------------------------------------------------------------//

// Note: Most of the code here is a spaghetti relic of the old Moonshot. 
// It'll be completly rewritten in the future. Same for Token.hpp/.cpp

#pragma once

#include "Token.hpp"
#include "Fox/Common/StringManipulator.hpp"

namespace fox {
  class DiagnosticEngine;
  class Lexer  {
    public:
      Lexer(ASTContext &astctxt);

      // Lexs a file in the SourceManager.
      // This will retrieve the file from the SourceManager from the current Context.
      void lexFile(FileID file);
  
      TokenVector& getTokenVector();
      std::size_t resultSize() const;  
      FileID getCurrentFile() const;

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
      void markBeginningOfToken(); // sets currentTokenBeginIndex_ to the current index

      void runStateFunc();
      void dfa_goto(DFAState ns);
      void fn_S_BASE();  // base state
      void fn_S_STR();  // string literals
      void fn_S_LCOM();  // one line comment
      void fn_S_MCOM();  // Multiple line comments
      void fn_S_WORDS();  // "basic" state (push to tok until separator is met)
      void fn_S_CHR();  // Char literals

      // Utils
      FoxChar eatChar();            // returns the current char and run updatePos (returns inputstr_[pos_] and do pos_+=1)
      void addToCurtok(FoxChar c);      // adds the current character to curtok
      bool isSep(FoxChar c) const;      // is the current char a separator? (= a sign. see kSign_dict)
      bool isEscapeChar(FoxChar c) const;  // Checks if C is \ AND if the state is adequate for it to be qualified as an escape char.
      bool shouldIgnore(FoxChar c) const;  // Checks if the char is valid to be pushed. If it isn't and it should be ignored, returns true

      SourceLoc getCurtokBegLoc() const;
      SourceRange getCurtokRange() const;

      ASTContext& ctxt_;
      DiagnosticEngine& diags_;
      FileID currentFile_;

      bool escapeFlag_ : 1;
      DFAState state_ = DFAState::S_BASE;    
      std::string curtok_;

      // The index of the first character of the current token being processed.
      SourceLoc::IndexTy currentTokenBeginIndex_;

      TokenVector  tokens_;
      StringManipulator manip_;
  };
}
