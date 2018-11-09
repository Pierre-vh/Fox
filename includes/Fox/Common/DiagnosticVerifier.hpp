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
#include "DiagnosticConsumers.hpp"
#include <set>

namespace fox {
  class Diagnostic;
  class DiagnosticEngine;
	template<typename Ty> class ResultObject;
  class DiagnosticVerifier : public DiagnosticConsumer{
    using LineTy = CompleteLoc::LineTy;
    public:
      // Creates a DiagnosticVerifier. A DV will always require a consumer
      // attached to it, the value cannot be nullptr.
      DiagnosticVerifier(DiagnosticEngine& engine, SourceManager& srcMgr);

      // Parses a file, searching for "expect-" directives, parsing them and 
      // adding them to the list of expected diagnostics.
      // Returns true if the file was parsed successfully (no diags emitted),
      // false otherwise.
      bool parseFile(FileID file);

      virtual void consume(Diagnostic& diag) override;

    private:
			struct VerifyInstr {
				VerifyInstr() = default;

				VerifyInstr(string_view suffix, std::int8_t offset, string_view str):
					suffix(suffix), offset(offset), str(str) {}

				string_view suffix;
				std::int8_t offset = 0;
				string_view str;
			};

      void addExpectedDiag(FileID file, LineTy line, string_view str);

      // Handles a verify instr, parsing it and processing it.
      // The first argument is the loc of the first char of the instr.
      bool handleVerifyInstr(SourceLoc loc, string_view instr);

			// Parses a verify instr
			ResultObject<VerifyInstr> parseVerifyInstr(SourceLoc loc,
																								 string_view instr);

			void diagnoseMissingStr(SourceLoc loc);
			void diagnoseMissingColon(SourceLoc loc);
      void diagnoseMissingSuffix(SourceLoc instrBeg);

      DiagnosticEngine& diags_;
      SourceManager& srcMgr_;
      // Map of expected diagnostics
      std::multimap<string_view, std::pair<FileID, LineTy>> expectedDiags_;
  };
} // namespace fox
