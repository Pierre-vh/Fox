//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : DiagnosticEngine.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file declares the Diagnostic class (which stores 
// informations on a specific diagnostic).
// This file also contains the DiagID enum, which store every possible
// diagnostics
//----------------------------------------------------------------------------//

#pragma once

#include "Source.hpp"
#include "StringManipulator.hpp"
#include <string>
#include <sstream>

namespace fox {
  class DiagnosticEngine;
  enum class DiagID : std::uint16_t {
    // Important : first value must always be 0 to keep sync
    // with the severities and strs arrays.
    #define DIAG(SEVERITY,ID,TEXT) ID,
    #define DIAG_RANGE(ID,FIRST,LAST) First_##ID = FIRST, Last_##ID = LAST,
    #include "Diags/All.def"
  };

  enum class DiagSeverity : std::uint8_t {
    IGNORE,
    NOTE,
    WARNING,
    ERROR,  
    FATAL    
  };

  class Diagnostic {
    // Note: in this class, some methods will return a Diagnostic&. 
    // This is done to enable function chaining.
    // e.g. someDiag.addArg(..).addArg(...).freeze()
    protected:
      friend class DiagnosticEngine;

      Diagnostic(DiagnosticEngine *engine, DiagID dID, DiagSeverity dSev,
        const std::string& dStr, const SourceRange& range = SourceRange());
    
    public:
      // Note : both copy/move ctors kill the copied diag.
      Diagnostic(Diagnostic &other);
      Diagnostic(Diagnostic &&other);

      // Destructor that emits the diag.
      ~Diagnostic();
      
      void emit();

      // Sets this diagnostic's severity to Ignore.
      void ignore();

      // Getters for basic args values
      DiagID getID() const;
      std::string getStr() const;
      DiagSeverity getSeverity() const;

      SourceRange getRange() const;
      Diagnostic& setRange(SourceRange range);
      bool hasRange() const;

      SourceRange getExtraRange() const;
      Diagnostic& setExtraRange(SourceRange range);
      bool hasExtraRange() const;

      // File-wide diagnostics are diagnostics that concern
      // a whole file. 
      Diagnostic& setIsFileWide(bool fileWide);
      bool isFileWide() const;

      // Replace a %x placeholder.
      template<typename ReplTy>
      inline Diagnostic& addArg(const ReplTy& value) {
        auto tmp = curPHIndex_;
        curPHIndex_++;
        return addArg(value,tmp);
      }

      // Frozen diags are locked, they cannot be modified further.
      bool isFrozen() const;
      Diagnostic& freeze();

      // Inactive diags won't be emitted.
      bool isActive() const;
      explicit operator bool() const;

    private:
      friend class DiagnosticEngine;
      Diagnostic& operator=(const Diagnostic&) = default;

      // Internal addArg overloads
      template<typename ReplTy>
      inline Diagnostic& addArg(const ReplTy& value,
        std::uint8_t phIndex) {
        std::stringstream ss;
        ss << value;
        return replacePlaceholder(ss.str(), phIndex);
      }

      // For std::strings
      template<>
      inline Diagnostic& addArg(const std::string& value, 
        std::uint8_t phIndex) {
        return replacePlaceholder(value, phIndex);
      }

      // for FoxChar
      template<>
      inline Diagnostic& addArg(const FoxChar& value,
        std::uint8_t phIndex) {
        return replacePlaceholder(
          StringManipulator::charToStr(value), phIndex
        );
      }

      // replaces every occurence of "%(value of index)" 
      // in a string with the replacement value
      // e.g: replacePlaceholder("foo",0) replaces every %0 
      // in the string with "foo"
      Diagnostic& replacePlaceholder(const std::string& replacement,
        std::uint8_t index);

      void kill(); 
      
      void initBitFields();  

      // Packed in 8 bits (0 left)
      bool active_ :1; 
      bool frozen_ :1; 
      std::uint8_t curPHIndex_ :6;

      // Packed in 8 bits (3 left)
      DiagSeverity diagSeverity_ : 4; 
      bool fileWide_ : 1;

      DiagnosticEngine* engine_ = nullptr;
      DiagID diagID_;
      std::string diagStr_;
      SourceRange range_;
      SourceRange extraRange_;
  };
}
