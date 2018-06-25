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

#include "DiagnosticConsumers.hpp"
#include "Moonshot/Fox/Common/Utils.hpp"

#include <memory>

#define DIAGENGINE_DEFAULT_ERR_LIMIT 0

namespace Moonshot
{
	class Diagnostic;
	enum class DiagSeverity : int8_t;
	enum class DiagID : int16_t;
	class SourceLoc;
	class SourceRange;
	class DiagnosticEngine
	{
		public:
			// Constructor that will use a default DiagnosticConsumer
			DiagnosticEngine(SourceManager& sm);

			// Constructor that will use a pre-created DiagnosticConsumer
			DiagnosticEngine(std::unique_ptr<DiagnosticConsumer> ncons);

			Diagnostic report(const DiagID& diagID);
			Diagnostic report(const DiagID& diagID, const SourceRange& range);
			Diagnostic report(const DiagID& diagID, const SourceLoc& loc);

			void setConsumer(std::unique_ptr<DiagnosticConsumer> ncons);
			DiagnosticConsumer * getConsumer();

			// Sets every option to false
			void resetAllOptions(); 

			// Returns true if a fatal errors has occured.
			bool hasFatalErrorOccured() const;

			// Getters for Number of warnings/non fatal errors that have occured.
			unsigned int getNumWarnings() const;
			unsigned int getNumErrors() const;

			// Get/Set max number of errors before a the context silences all future errors & warnings
			// and reports a "err_count_too_high" error.
			// Set to 0 for unlimited.
			unsigned int getErrorLimit() const;
			void setErrorLimit(const unsigned int& mErr);

			// Manual getters/setters for DiagOpts
			bool getWarningsAreErrors() const;
			void setWarningsAreErrors(const bool& val);

			bool getErrorsAreFatal() const;
			void setErrorsAreFatal(const bool& val);

			bool getSilenceWarnings() const;
			void setSilenceWarnings(const bool& val);

			bool getSilenceNotes() const;
			void setSilenceNotes(const bool& val);

			bool getSilenceAllAfterFatalErrors() const;
			void setSilenceAllAfterFatalErrors(const bool& val);

			bool getSilenceAll() const;
			void setSilenceAll(const bool& val);
		private:		
			// Promotes the severity of the diagnostic if needed
			DiagSeverity promoteSeverityIfNeeded(const DiagSeverity& ds) const;

			// returns true if the diagnostic should be silenced (not emitted)
			bool shouldSilence(const DiagSeverity& ds);

			// Updates internal counters (warningCount, errCount, hasFatalErrorOccured) depending on the severity
			void updateInternalCounters(const DiagSeverity& ds);

			// have too many errors occured
			bool haveTooManyErrorsOccured() const;

			// Member Variables //
			/* Options to change how the DiagEngine behaves when an error is reported */
			struct DiagOpts
			{
				// Promotion of severities
				bool warningsAreErrors	: 1;
				bool errorsAreFatal : 1;

				// Silence Warnings/Notes
				bool silenceWarnings	: 1;
				bool silenceNotes		: 1;
				bool silenceAllAfterFatalError : 1;
				bool silenceAll : 1;
			} diagOpts_;

			/* Other non bool parameters */
			unsigned int errLimit_ = DIAGENGINE_DEFAULT_ERR_LIMIT; 

			/* Statistics */
			unsigned int errorCount_		= 0;
			unsigned int warnCount_	= 0;
			bool hasFatalErrorOccured_ = false;
			bool hasReportedErrLimitExceededError_ = false;			// This flag is set to true when the diagengine has emitted the "max error" fatal error.

			std::unique_ptr<DiagnosticConsumer> consumer_	= nullptr;
	};
}