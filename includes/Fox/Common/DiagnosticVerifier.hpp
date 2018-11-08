//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.
// See the LICENSE.txt file at the root of the project for license information.
// File : DiagnosticVerifier.hpp
// Author : Pierre van Houtryve
//----------------------------------------------------------------------------//
// This file contains the DiagnosticVerifier diagnostic consumer class.
// The DV offers a tools to parse a file, finding every "expect-" instruction
// to silence expected diagnostics in tests, and doubles as a DiagnosticConsumer
// class which catches every diagnostic, checks if it was expected, and if
// that's the case, silences it.
//----------------------------------------------------------------------------//

#pragma once

#include "Source.hpp"
#include <set>

namespace fox {
  class Diagnostic;
  class DiagnosticVerifier {
    using LineTy = CompleteLoc::LineTy;
    public:
      // Creates a DiagnosticVerifier. A DV will always require a consumer
      // attached to it, the value cannot be nullptr.
      DiagnosticVerifier(SourceManager& srcMgr);

      // Parses a file, searching for "expect-" directives, parsing them and 
      // adding them to the list of expected diagnostics.
      // Returns true if the file was parsed successfully (no diags emitted),
      // false otherwise.
      bool parseFile(FileID file);

    protected:
      friend class DiagnosticEngine;
      // Called by the DiagnosticEngine when it desires to Verify a diagnostic.
      // Returns true if the Diagnostic should be emitted, false otherwise.
      bool verify(Diagnostic& diag);

      void addExpectedDiag(FileID file, LineTy line, string_view str);
    private:
      SourceManager& srcMgr_;
      // Map of expected diagnostics
      std::multimap<string_view, std::pair<FileID, LineTy>> expectedDiags_;
  };
} // namespace fox
