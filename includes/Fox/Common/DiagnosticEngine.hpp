//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : DiagnosticEngine.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the classes related to diagnostics emission in Fox.
//----------------------------------------------------------------------------//

#pragma once

#include "DiagnosticConsumers.hpp"
#include "SourceLoc.hpp"
#include "string_view.hpp"
#include "FoxTypes.hpp"
#include <string>
#include <sstream>
#include <memory>

namespace fox {
  class SourceManager;
  class DiagnosticVerifier;
  class Diagnostic;
  class DiagnosticEngine;

  /// Diagnostic ID/Kinds
  enum class DiagID : std::uint16_t {
    // Important : first value must always be 0 to keep sync
    // with the severities and strs arrays.
    #define DIAG(SEVERITY,ID,TEXT) ID,
    #include "Diags/All.def"
  };

  /// Diagnostic Severities 
  enum class DiagSeverity : std::uint8_t {
    Ignore, Note, Warning, Error, Fatal    
    // Note: There's still room for 3 more severities.
    // If more are added, add an extra bit to diagSeverity_ in
    // the Diagnostic class
  };

  /// Converts a severity to a user readable string.
  std::string toString(DiagSeverity sev);
  std::ostream& operator<<(std::ostream& os, DiagSeverity sev);

  /// The Diagnostic object. It contains the Diagnostic's data and allow
  /// the client to customize it before emitting it.
  class Diagnostic {
    friend class DiagnosticEngine;

    Diagnostic(DiagnosticEngine *engine, DiagID dID, DiagSeverity dSev,
      string_view dStr, SourceRange range, bool isFileWide);
    
    public:
      // Only allow moving the diagnostic, not copying it.
      Diagnostic(Diagnostic &&other);
      Diagnostic(Diagnostic&) = delete;
      Diagnostic& operator=(Diagnostic&) = delete;
      Diagnostic& operator=(Diagnostic&& other);
     
      /// Dtor that emits the diagnostic.
      ~Diagnostic();
      
      /// Emit this diagnostic, feeding it to the consumer and killing it
      /// once emitted.
      void emit();

      /// \returns the DiagID of this diagnostic.
      DiagID getID() const;

      /// \returns the string of this diagnostic in its current form.
      std::string getStr() const;

      /// \returns this diagnostic's severity.
      DiagSeverity getSeverity() const;

      /// \returns the FileID of the file concerned by this diagnostic
      FileID getFileID() const;

      SourceRange getSourceRange() const;
      Diagnostic& setRange(SourceRange range);

      /// \returns true if this diagnostic contains precise location
      /// information
      bool hasPreciseLoc() const;
      /// \returns true if this Diagnostic contains an additional location
      /// information.
      bool hasExtraLoc() const;

      SourceRange getExtraRange() const;
      Diagnostic& setExtraRange(SourceRange range);

      /// \returns true if this Diagnostic contains any kind of source location
      /// information, file-wide or not.
      bool hasAnyLocInfo() const;

      /// \returns true if this diagnostic is "file-wide" (it concerns a whole file)
      bool isFileWide() const;

      // addArg Implementation for any type that supports operator <<
      template<typename ReplTy>
      Diagnostic& addArg(const ReplTy& value) {
        std::stringstream ss;
        ss << value;
        return replacePlaceholder(ss.str());
      }

      template<>
      Diagnostic& addArg(const std::string& value) {
        return replacePlaceholder(value);
      }

      template<>
      Diagnostic& addArg(const string_view& value) {
        return replacePlaceholder(value.to_string());
      }

      template<>
      Diagnostic& addArg(const FoxChar& value) {
        return replacePlaceholder(value);
      }

      /// \returns true if this Diagnostic is active, false otherwise.
      ///
      /// Active diags can be augmented & emitted, while inactives ones can't.
      bool isActive() const;

      /// \returns isActive()
      explicit operator bool() const;

    private:
      // Some static asserts can't be done inside the header, so this helper 
      // class is used to access the private data of the Diagnostic object
      // inside the .cpp without being in a function.
      class StaticAsserts;
      
      // replaces every occurence of "%(value of index)" 
      // in a string with the replacement value
      // e.g: replacePlaceholder("foo",0) replaces every %0 
      //      in the string with "foo"
      Diagnostic& replacePlaceholder(string_view replacement);
      Diagnostic& replacePlaceholder(FoxChar replacement);

      // Kills this diagnostic, removing most of it's data and
      // deactivating it.
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

  /// The DiagnosticEngine controls the creation and emission of diagnostics.
  class DiagnosticEngine {
    public:
      /// Constructor that will use the default Diagnostic Consumer
      /// which prints pretty-printed diagnostics to \p os
      DiagnosticEngine(SourceManager& sm, std::ostream& os);

      /// Make this class non copyable
      DiagnosticEngine(const DiagnosticEngine&) = delete;
      DiagnosticEngine& operator=(const DiagnosticEngine&) = delete;

      /// Constructor for when you want to use a pre-created DiagnosticConsumer
      DiagnosticEngine(SourceManager& sm, 
                       std::unique_ptr<DiagnosticConsumer> ncons);

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

      /// \returns true if a fatal error was emitted
      bool hadFatalError() const;

      /// \returns true if any error, fatal or not, was emitted.
      bool hadAnyError() const;

      bool getWarningsAreErrors() const;
      void setWarningsAreErrors(bool val);

      bool getIgnoreWarnings() const;
      void setIgnoreWarnings(bool val);

      bool getIgnoreNotes() const;
      void setIgnoreNotes(bool val);

      bool getIgnoreAllAfterFatal() const;
      void setIgnoreAllAfterFatal(bool val);

      bool getIgnoreAll() const;
      void setIgnoreAll(bool val);

      SourceManager& srcMgr;

    private:
      friend class Diagnostic;

      // Internal overload of 'report'. Every public 'report'
      // function calls this one.
      Diagnostic report(DiagID diagID, SourceRange range, bool isFileWide);

      // Called by Diagnostic::emit
      void handleDiagnostic(Diagnostic& diag);

      // Promotes the severity of the diagnostic if needed
      DiagSeverity changeSeverityIfNeeded(DiagSeverity ds) const;

      // Updates the internal state depending on the severity of an
      // emitted diagnostic
      void updateInternalState(DiagSeverity ds);

      // Bitfields : Options
      bool warningsAreErrors_  : 1;
      bool ignoreWarnings_ : 1;
      bool ignoreNotes_ : 1;
      bool ignoreAllAfterFatalError_ : 1;
      bool ignoreAll_ : 1;
      bool hadFatalError_ : 1;
      bool hadError_ : 1;
      // 1 bit left

      // The DiagnosticVerifier, if there's one
      DiagnosticVerifier* verifier_ = nullptr;

      // The DiagnosticConsumer
      std::unique_ptr<DiagnosticConsumer> consumer_;
  };
}
