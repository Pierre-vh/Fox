////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : DiagnosticEngine.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "DiagnosticEngine.hpp"
#include "Moonshot/Common/Flags/FlagsManager.hpp"
#include "Diagnostic.hpp"
#include "BuiltinDiagConsumers.hpp"
#include <cassert>

using namespace Moonshot;

static const char* diagsStrs[] = {
	#define DIAG(SEVERITY,ID,TEXT) TEXT,
	#define KEEP_DIAG_DEF
		#include "Diags/DiagsAll.def"
	#undef DIAG
	#undef KEEP_DIAG_DEF
};

static const DiagSeverity diagsSevs[] = {
	#define DIAG(SEVERITY,ID,TEXT) DiagSeverity::SEVERITY,
	#define KEEP_DIAG_DEF
		#include "Diags/DiagsAll.def"
	#undef DIAG
	#undef KEEP_DIAG_DEF
};

DiagnosticEngine::DiagnosticEngine(FlagsManager *fm) : flagsManager_(fm)
{
	consumer_ = std::make_unique<StreamDiagConsumer>(); // Default diag consumer outputs to cout

	setupDiagOpts();
}

DiagnosticEngine::DiagnosticEngine(std::unique_ptr<IDiagConsumer> ncons,FlagsManager *fm): consumer_(std::move(ncons)), flagsManager_(fm)
{
	setupDiagOpts();
}

Diagnostic DiagnosticEngine::report(const DiagID & diagID)
{
	assert((bool)consumer_ && "No consumer available!");
	// Gather diagnostic info
	const auto idx = Util::enumAsInt(diagID);
	DiagSeverity sev = diagsSevs[idx];
	std::string str(diagsStrs[idx]);
	
	// Promote severity if needed
	sev = promoteSeverityIfNeeded(sev);

	// We test if we need to silence this diag before updating the internal counters.
	// This is needed so this diag isn't counted in the current statistics, and only the other 
	// Diags stats are take in considerations.
	bool silence = shouldSilence(sev);

	// Update the internal counters
	updateInternalCounters(sev);

	// Silence diag if needed
	if (silence)
		return Diagnostic::createDummyDiagnosticObject();
	else if (haveTooManyErrorOccured() && (!hasFatalErrorOccured())) 	// If the diag shouldn't be silenced, check if it needs to be overriden by a maxErrCount diag
	{
		// if the error count is too high and we're not in fatal mode yet, instead of the user requested diag, we return a maxErrOccured diag
		return report(DiagID::diagengine_maxErrCountExceeded).addArg(errLimit_).freeze(); /* Freeze the diagnostic to prevent user modifications */
	}
	else	// If we're here, fine, return the user requested diag!
	{
		return Diagnostic(
			consumer_.get(),
			diagID,
			sev,
			str
		);
	}
}

void DiagnosticEngine::setConsumer(std::unique_ptr<IDiagConsumer> ncons)
{
	consumer_ = std::move(ncons);
}

IDiagConsumer* DiagnosticEngine::getConsumer()
{
	return consumer_.get();
}

void DiagnosticEngine::setFlagsManager(FlagsManager * fm)
{
	flagsManager_ = fm;
	if(fm)
		updateOptionsFromFlags();
}

FlagsManager * const DiagnosticEngine::getFlagsManager()
{
	return flagsManager_;
}

bool DiagnosticEngine::updateOptionsFromFlags()
{
	if (flagsManager_)
	{
		diagOpts_.errorsAreFatal = flagsManager_->isSet(FlagID::diagengine_errorsAreFatal);
		diagOpts_.silenceAll = flagsManager_->isSet(FlagID::diagengine_silenceAll);
		diagOpts_.silenceAllAfterFatalError = flagsManager_->isSet(FlagID::diagengine_silenceAllAfterFatalError);
		diagOpts_.silenceNotes = flagsManager_->isSet(FlagID::diagengine_silenceNotes);
		diagOpts_.silenceWarnings = flagsManager_->isSet(FlagID::diagengine_silenceWarnings);
		diagOpts_.warningsAreErrors = flagsManager_->isSet(FlagID::diagengine_warningsAreErrors);
	}
	return (bool)flagsManager_;
}

void DiagnosticEngine::resetAllOptions()
{
	diagOpts_.errorsAreFatal			= false;
	diagOpts_.silenceAll				= false;
	diagOpts_.silenceAllAfterFatalError = false;
	diagOpts_.silenceNotes				= false;
	diagOpts_.silenceWarnings			= false;
	diagOpts_.warningsAreErrors			= false;
}

bool DiagnosticEngine::hasFatalErrorOccured() const
{
	return hasFatalErrorOccured_;
}

unsigned int DiagnosticEngine::getNumWarnings() const
{
	return numWarnings_;
}

unsigned int DiagnosticEngine::getNumErrors() const
{
	return numErrors_;
}

unsigned int DiagnosticEngine::getErrorLimit() const
{
	return errLimit_;
}

void DiagnosticEngine::setErrorLimit(const unsigned int & mErr)
{
	errLimit_ = mErr;
}

bool DiagnosticEngine::getWarningsAreErrors() const
{
	return diagOpts_.warningsAreErrors;
}

void DiagnosticEngine::setWarningsAreErrors(const bool & val)
{
	diagOpts_.warningsAreErrors = val;
}

bool DiagnosticEngine::getErrorsAreFatal() const
{
	return diagOpts_.errorsAreFatal;
}

void DiagnosticEngine::setErrorsAreFatal(const bool & val)
{
	diagOpts_.errorsAreFatal = val;
}

bool DiagnosticEngine::getSilenceWarnings() const
{
	return diagOpts_.silenceWarnings;
}

void DiagnosticEngine::setSilenceWarnings(const bool & val)
{
	diagOpts_.silenceWarnings = val;
}

bool DiagnosticEngine::getSilenceNotes() const
{
	return diagOpts_.silenceNotes;
}

void DiagnosticEngine::setSilenceNotes(const bool & val)
{
	diagOpts_.silenceNotes = val;
}

bool DiagnosticEngine::getSilenceAllAfterFatalErrors() const
{
	return diagOpts_.silenceAllAfterFatalError;
}

void DiagnosticEngine::setSilenceAllAfterFatalErrors(const bool & val)
{
	diagOpts_.silenceAllAfterFatalError = val;
}

bool DiagnosticEngine::getSilenceAll() const
{
	return diagOpts_.silenceAll;
}

void DiagnosticEngine::setSilenceAll(const bool & val)
{
	diagOpts_.silenceAll = val;
}

void DiagnosticEngine::setupDiagOpts()
{
	if (flagsManager_)
		updateOptionsFromFlags();
	else
		resetAllOptions();
}

DiagSeverity DiagnosticEngine::promoteSeverityIfNeeded(const DiagSeverity & ds) const
{
	switch (ds)
	{
		case DiagSeverity::IGNORE:
		case DiagSeverity::NOTE:
			return ds;
		case DiagSeverity::WARNING:
			if (getWarningsAreErrors())
				return DiagSeverity::ERROR;
			else
				return ds;
		case DiagSeverity::ERROR:
			if (getErrorsAreFatal())
				return DiagSeverity::FATAL;
			else
				return ds;
		case DiagSeverity::FATAL:
			return ds;
	}
	return ds;
}

bool DiagnosticEngine::shouldSilence(const DiagSeverity & df)
{
	// Don't emit any diagnostic if silenceAll is set
	if (getSilenceAll())
		return true;
	// Don't emit any diagnostic if a fatal error occured and silenceAllAfterFatalError is set
	if (getSilenceAllAfterFatalErrors() && hasFatalErrorOccured())
		return true;
	// If the diagnostic shouldn't be silenced :
	switch (df)
	{
		// Ignored Diagnostics are never emitted
		case DiagSeverity::IGNORE:
			return true;
		// Notes are not emitted if silenceNotes is active
		case DiagSeverity::NOTE:
			return getSilenceNotes();
		// Warnings are not emitted if silenceWarnings is active
		case DiagSeverity::WARNING:
			return getSilenceWarnings();
		// Errors are no longer emitted if too many errors have occured
		case DiagSeverity::ERROR:
			return haveTooManyErrorOccured();
		// Severe diagnostics are never ignored 
		case DiagSeverity::FATAL:
			return false;
	}
	return false;
}

void DiagnosticEngine::updateInternalCounters(const DiagSeverity & ds)
{
	switch (ds)
	{
		case DiagSeverity::WARNING:
			numWarnings_++;
			break;
		case DiagSeverity::ERROR:
			numErrors_++;
			break;
		case DiagSeverity::FATAL:
			hasFatalErrorOccured_ = true;
			break;
	}
}

bool DiagnosticEngine::haveTooManyErrorOccured() const
{
	if(errLimit_)
		return numErrors_ > errLimit_;
	return false;
}