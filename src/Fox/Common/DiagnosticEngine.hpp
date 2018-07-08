////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : DiagnosticEngine.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the DiagnosticEngine class, which is used
// to coordinate diagnostic generation and consumption.
////------------------------------------------------------////

#pragma once

#include <memory>
#include "Diagnostic.hpp"
#include "DiagnosticConsumers.hpp"
#define DIAGENGINE_DEFAULT_ERR_LIMIT 0

namespace fox
{
	class SourceLoc;
	class SourceRange;
	class SourceManager;
	class DiagnosticEngine
	{
		public:
			// Constructor that will use a default DiagnosticConsumer
			DiagnosticEngine(SourceManager* sm = nullptr);

			// Constructor that will use a pre-created DiagnosticConsumer
			DiagnosticEngine(std::unique_ptr<DiagnosticConsumer> ncons);

			Diagnostic report(DiagID diagID);
			Diagnostic report(DiagID diagID, const FileID& file);
			Diagnostic report(DiagID diagID, const SourceRange& range);
			Diagnostic report(DiagID diagID, const SourceLoc& loc);

			void setConsumer(std::unique_ptr<DiagnosticConsumer> ncons);
			DiagnosticConsumer * getConsumer();

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

			bool getSilenceWarnings() const;
			void setSilenceWarnings(bool val);

			bool getSilenceNotes() const;
			void setSilenceNotes(bool val);

			bool getSilenceAllAfterFatalErrors() const;
			void setSilenceAllAfterFatalErrors(bool val);

			bool getSilenceAll() const;
			void setSilenceAll(bool val);

		protected:
			friend class Diagnostic;
			void handleDiagnostic(Diagnostic& diag);

		private:		
			// Promotes the severity of the diagnostic if needed
			DiagSeverity changeSeverityIfNeeded(DiagSeverity ds) const;

			// Updates internal counters (warningCount, errCount, hasFatalErrorOccured) depending on the severity
			void updateInternalCounters(DiagSeverity ds);

			// Bitfields : Options
			bool warningsAreErrors_	: 1;
			bool errorsAreFatal_	: 1;
			bool silenceWarnings_	: 1;
			bool silenceNotes_		: 1;
			bool silenceAllAfterFatalError_ : 1;
			bool silenceAll_ : 1;
			// 2 bits left : Flags
			bool hasFatalErrorOccured_	: 1;
			bool errLimitReached_		: 1;

			/* Other non bool parameters */
			std::uint16_t errLimit_ = DIAGENGINE_DEFAULT_ERR_LIMIT;

			/* Statistics */
			std::uint16_t errorCount_ = 0;
			std::uint16_t warnCount_  = 0;

			std::unique_ptr<DiagnosticConsumer> consumer_;
	};
}