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

#include <memory>
#include "Diagnostic.hpp"
#include "DiagnosticConsumers.hpp"

namespace fox {
  class SourceLoc;
  class SourceRange;
  class SourceManager;
  class DiagnosticVerifier;
  class DiagnosticEngine {
    public:
      // Constructor that will use the default Diagnostic Consumer
      // which prints pretty-printed diagnostics to the desired ostream.
      DiagnosticEngine(SourceManager& sm, std::ostream& os = std::cout);

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
}
