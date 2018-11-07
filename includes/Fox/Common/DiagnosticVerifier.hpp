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

#include "DiagnosticConsumers.hpp"
#include "string_view.hpp"
#include "Source.hpp"
#include <set>

namespace fox {
  class DiagnosticEngine;
  class DiagnosticVerifier : public DiagnosticConsumer {
    using LineTy = CompleteLoc::LineTy;
    public:
      struct ExpectedDiag {
        string_view str;
        FileID file;
        LineTy line;
      };
      using DiagnosticSetTy = std::multiset<string_view>;

      // Creates a DiagnosticVerifier. A DV will always require a consumer
      // attached to it, the value cannot be nullptr.
      // The DV will also require a DiagnosticEngine attached to it to
      // emit it's own diagnostics.
      DiagnosticVerifier(SourceManager& srcMgr, std::unique_ptr<DiagnosticConsumer> consumer);

      virtual void consume(Diagnostic& diagnostic) override;

      // Parses a file, searching for "expect-" directives, parsing them and 
      // adding them to the list of expected diagnostics.
      // Returns true if the file was parsed successfully (no diags emitted),
      // false otherwise.
      bool parseFile(FileID file);

    private:
      SourceManager& srcMgr_;
      std::unique_ptr<DiagnosticConsumer> consumer_;
      std::multiset<string_view> expectedDiags_;
  };
} // namespace fox
