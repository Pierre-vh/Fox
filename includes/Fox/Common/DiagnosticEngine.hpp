//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : DiagnosticEngine.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file declares the DiagnosticEngine class, which is used
// to coordinate diagnostic generation and consumption.
//
// This class will also promote or demote diagnostics severity 
// based on some options. It will also keep track of how many errors
// and warnings have been emitted.
//----------------------------------------------------------------------------//

#pragma once

#include "DiagnosticConsumers.hpp"
#include "Source.hpp"
#include "StringManipulator.hpp"
#include <string>
#include <sstream>
#include <memory>

namespace fox {
  class SourceLoc;
  class SourceRange;
  class SourceManager;
  class DiagnosticVerifier;
  class Diagnostic;

  // Diagnostic ID/Kinds
  enum class DiagID : std::uint16_t {
    // Important : first value must always be 0 to keep sync
    // with the severities and strs arrays.
    #define DIAG(SEVERITY,ID,TEXT) ID,
    #define DIAG_RANGE(ID,FIRST,LAST) First_##ID = FIRST, Last_##ID = LAST,
    #include "Diags/All.def"
  };

  // Diagnostic Severities 
  enum class DiagSeverity : std::uint8_t {
    Ignore, Note, Warning, Error, Fatal    
  };

  // Converts a severity to a user readable string. 
  // If allCaps is set to true, the returned value will be in all caps.
  std::string toString(DiagSeverity sev, bool allCaps = false);
  std::ostream& operator<<(std::ostream& os, DiagSeverity sev);

  // The DiagnosticEngine, which controls everything Diagnostic-Related:
  //  Creation of Diagnostics, Emission (but not presentation, see
  //  DiagnosticConsumer for that), silencing/promoting diagnostics, etc.
  class DiagnosticEngine {
    public:
      // Constructor that will use the default Diagnostic Consumer
      // which prints pretty-printed diagnostics to the desired ostream.
      DiagnosticEngine(SourceManager& sm, std::ostream& os);

      // Constructor that will use the default Diagnostic Consumer
      // which prints pretty-printed diagnostics to std::cout
      DiagnosticEngine(SourceManager& sm);

      // Constructor that will use a pre-created DiagnosticConsumer
      DiagnosticEngine(std::unique_ptr<DiagnosticConsumer> ncons);

      Diagnostic report(DiagID diagID);
      Diagnostic report(DiagID diagID, FileID file);
      Diagnostic report(DiagID diagID, SourceRange range);
      Diagnostic report(DiagID diagID, SourceLoc loc);

      void enableVerifyMode(DiagnosticVerifier* dv);
      bool isVerifyModeEnabled() const;
      void disableVerifyMode();

      void setConsumer(std::unique_ptr<DiagnosticConsumer> ncons);
      DiagnosticConsumer* getConsumer();
      const DiagnosticConsumer* getConsumer() const;
      std::unique_ptr<DiagnosticConsumer> takeConsumer();

      // Returns true if a fatal errors has been emitted.
      bool hasFatalErrorOccured() const;

      // Getters for Number of warnings/errors that have been emitted.
      std::uint16_t getWarningsCount() const;
      std::uint16_t getErrorsCount() const;

      // Set the max number of errors that can occur
      // before a the context silences all future diagnostics
      // and reports a fatal "Too many errors" error.
      // Set to 0 for unlimited.
      void setErrorLimit(std::uint16_t mErr);
      std::uint16_t getErrorLimit() const;

      bool getWarningsAreErrors() const;
      void setWarningsAreErrors(bool val);

      bool getErrorsAreFatal() const;
      void setErrorsAreFatal(bool val);

      bool getIgnoreWarnings() const;
      void setIgnoreWarnings(bool val);

      bool getIgnoreNotes() const;
      void setIgnoreNotes(bool val);

      bool getIgnoreAllAfterFatal() const;
      void setIgnoreAllAfterFatal(bool val);

      bool getIgnoreAll() const;
      void setIgnoreAll(bool val);

      static constexpr std::uint16_t defaultErrorLimit = 0;

    protected:
      friend class Diagnostic;

      // Called by the Diagnostic's destructor. This will handle
      // the emission of the diagnostic.
      void handleDiagnostic(Diagnostic& diag);

    private:    
      // Promotes the severity of the diagnostic if needed
      DiagSeverity changeSeverityIfNeeded(DiagSeverity ds) const;

      // Updates internal counters (warningCount, errCount, hasFatalErrorOccured) depending on the severity
      void updateInternalCounters(DiagSeverity ds);

      // Bitfields : Options
      bool warningsAreErrors_  : 1;
      bool errorsAreFatal_ : 1;
      bool ignoreWarnings_ : 1;
      bool ignoreNotes_ : 1;
      bool ignoreAllAfterFatalError_ : 1;
      bool ignoreAll_ : 1;
      bool hasFatalErrorOccured_ : 1;
      bool errLimitReached_ : 1;
      // 0 bits left

      /* Other non bool parameters */
      std::uint16_t errLimit_ = defaultErrorLimit;

      /* Statistics */
      std::uint16_t errorCount_ = 0;
      std::uint16_t warnCount_  = 0;

      DiagnosticVerifier* verifier_ = nullptr;
      std::unique_ptr<DiagnosticConsumer> consumer_;
  };

  // The Diagnostic object. It contains the Diagnostic's data and allow
  // the client to customize it before emitting it.
  //
  // Note: in this class, some methods will return a Diagnostic&. 
  // This is done to enable function chaining.
  // e.g. someDiag.addArg(..).addArg(...).freeze()
  class Diagnostic {
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

      // TODO: Pack this better:
      //  Use PointerIntPair to do pack the "frozen" flag inside the DiagnosticEngine*
      //    for isActive, check if it still has a diagnosticEngine.
      //  Use a bitfield for the DiagID, 
      //    9 or 10 bits should be more than enough (static_assert it)
      //  Use 3 or 4 bits for curPhiIndex instead of 6 
      //    (assert that we don't exceed the max in replacePlaceholder)
      //  Use 3 bits for diagSeverity (static_assert it)

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
