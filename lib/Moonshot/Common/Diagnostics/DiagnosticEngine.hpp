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

#include "IDiagConsumer.hpp"
#include "Moonshot/Common/Utils/Utils.hpp"

#include <memory>

#define DIAGENGINE_DEFAULT_ERR_LIMIT 0

namespace Moonshot
{
	class Diagnostic;
	class FlagsManager;
	enum class DiagSeverity : int8_t;
	enum class DiagID : int16_t;
	class DiagnosticEngine
	{
		public:
			DiagnosticEngine(FlagsManager *fm = nullptr);
			DiagnosticEngine(std::unique_ptr<IDiagConsumer> ncons, FlagsManager *fm = nullptr);

			Diagnostic report(const DiagID& diagID);

			void setConsumer(std::unique_ptr<IDiagConsumer> ncons);
			IDiagConsumer * getConsumer();

			void setFlagsManager(FlagsManager *fm);
		    FlagsManager * const getFlagsManager();

			// Update the DiagOpts from the corresponding flags status (set/unset)
			// Returns true if the flagmanager was available and the operation was a success.
			bool updateOptionsFromFlags();
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
			// Calls updateOptionsFromFlags if a flagmanager is available, else, calls resetAllOptions()
			void setupDiagOpts();
		
			// Promotes the severity of the diagnostic if needed
			DiagSeverity promoteSeverityIfNeeded(const DiagSeverity& ds) const;

			// returns true if the diagnostic should be silenced (not emitted)
			bool shouldSilence(const DiagSeverity& ds);

			// Updates internal counters (warningCount, errCount, hasFatalErrorOccured) depending on the severity
			void updateInternalCounters(const DiagSeverity& ds);

			// have too many errors occured
			bool haveTooManyErrorOccured() const;

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
			unsigned int numErrors_		= 0;
			unsigned int numWarnings_	= 0;
			bool hasFatalErrorOccured_ = false;

			/* Observing pointer to a flagmanager, if there is one available */
			FlagsManager* flagsManager_					= nullptr;
			std::unique_ptr<IDiagConsumer> consumer_	= nullptr;
	};
}