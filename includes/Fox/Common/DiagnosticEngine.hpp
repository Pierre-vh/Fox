//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : DiagnosticEngine.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the classes related to diagnostics emission in Fox.
//----------------------------------------------------------------------------//

#pragma once

#include "DiagnosticConsumers.hpp"
#include "Source.hpp"
#include "Typedefs.hpp"
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
    #include "Diags/All.def"
  };

  // Diagnostic Severities 
  enum class DiagSeverity : std::uint8_t {
    Ignore, Note, Warning, Error, Fatal    
    // Note: There's still room for 3 more severities.
    // If more are added, add an extra bit to diagSeverity_ in
    // the Diagnostic class
  };

  // Converts a severity to a user readable string in all
  // lowercase.
  std::string toString(DiagSeverity sev);
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

    private:
      friend class Diagnostic;

      // Called by Diagnostic::emit
      void handleDiagnostic(Diagnostic& diag);

      // Promotes the severity of the diagnostic if needed
      DiagSeverity changeSeverityIfNeeded(DiagSeverity ds) const;

      // Updates internal counters depending on the severity of a diagnostic
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

      // Error limit
      std::uint16_t errLimit_ = defaultErrorLimit;
      // Number of errors
      std::uint16_t errorCount_ = 0;
      // Number of warnings
      std::uint16_t warnCount_  = 0;

      // The DiagnosticVerifier, if there's one
      DiagnosticVerifier* verifier_ = nullptr;

      // The DiagnosticConsumer
      std::unique_ptr<DiagnosticConsumer> consumer_;
  };

  // The Diagnostic object. It contains the Diagnostic's data and allow
  // the client to customize it before emitting it.
  //
  // Note: in this class, some methods will return a Diagnostic&. 
  // This is done to enable function chaining.
  // e.g. someDiag.addArg(..).addArg(...).freeze()
  class Diagnostic {
    friend class DiagnosticEngine;

    Diagnostic(DiagnosticEngine *engine, DiagID dID, DiagSeverity dSev,
      const std::string& dStr, const SourceRange& range = SourceRange());
    
    public:
      // Note : The copy constructor kills the copied diag.
      Diagnostic(Diagnostic &other);

      // Note : The move ctors kill the moved diag.
      Diagnostic(Diagnostic &&other);

      // Dtor that emits the diagnostic.
      ~Diagnostic();
      
      // Emit this diagnostic, feeding it to the consumer.
      void emit();

      // Returns the DiagID of this diagnostic.
      DiagID getID() const;

      // Returns the string of this diagnostic in it's current
      // form. The placeholders may or may not have been
      // removed!
      std::string getStr() const;

      // Returns this diagnostic's severity.
      DiagSeverity getSeverity() const;

      // Returns the FileID concerned by this diag.
      FileID getFileID() const;

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

      // addArg Implementation for any type that supports operator <<
      template<typename ReplTy>
      Diagnostic& addArg(const ReplTy& value) {
        std::stringstream ss;
        ss << value;
        return replacePlaceholder(ss.str());
      }

      // addArg Implementation for std::strings 
      template<>
      Diagnostic& addArg(const std::string& value) {
        return replacePlaceholder(value);
      }

      // addArg Implementation for string_view
      template<>
      Diagnostic& addArg(const string_view& value) {
        return replacePlaceholder(value.to_string());
      }

      // addArg Implementation for FoxChar
      template<>
      Diagnostic& addArg(const FoxChar& value) {
        return replacePlaceholder(value);
      }

      // Returns true if this Diagnostic is active, false otherwise.
      //
      // Active diagnostics can be modified & emitted, while
      // inactives ones can't.
      bool isActive() const;

      // isActive shortcut
      explicit operator bool() const;

    private:
      // Some static asserts can't be done inside the header, so this helper 
      // class is used to access the private data of the Diagnostic object
      // inside the .cpp without being in a function.
      class StaticAsserts;

      void initBitFields();  
      
      Diagnostic& operator=(const Diagnostic&) = default;

      // replaces every occurence of "%(value of index)" 
      // in a string with the replacement value
      // e.g: replacePlaceholder("foo",0) replaces every %0 
      // in the string with "foo"
      Diagnostic& replacePlaceholder(const std::string& replacement);
      Diagnostic& replacePlaceholder(FoxChar replacement);

      // Kills this diagnostic, removing most of it's data, thus
      // de-activating it.
      void kill(); 
      
      static constexpr unsigned placeholderIndexBits = 3;
      static constexpr unsigned diagIdBits = 9;

      //----------Packed in 16 bits (0 left)----------//
      // Placeholder index
      std::uint8_t curPHIndex_ : placeholderIndexBits; 
      // isFileWide flag
      bool fileWide_ : 1;
      // Severity of the Diagnostic
      DiagSeverity diagSeverity_ : 3; 
      // Kind of the diagnostic
      DiagID diagID_ : diagIdBits;
      //---------------------------------------------//

      DiagnosticEngine* engine_ = nullptr;

      std::string diagStr_;
      SourceRange range_;
      SourceRange extraRange_;
  };
}
