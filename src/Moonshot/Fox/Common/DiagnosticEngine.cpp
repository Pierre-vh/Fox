////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : DiagnosticEngine.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "DiagnosticEngine.hpp"

#include "Moonshot/Fox/Common/FlagsManager.hpp"

#include "Diagnostic.hpp"
#include "DiagnosticConsumers.hpp"

#include <cassert>

using namespace Moonshot;

static const char* diagsStrs[] = {
	#define DIAG(SEVERITY,ID,TEXT) TEXT,
		#include "Diags/DiagsAll.def"
};

static const DiagSeverity diagsSevs[] = {
	#define DIAG(SEVERITY,ID,TEXT) DiagSeverity::SEVERITY,
		#include "Diags/DiagsAll.def"
};

DiagnosticEngine::DiagnosticEngine(FlagsManager *fm) : flagsManager_(fm)
{
	consumer_ = std::make_unique<StreamDiagConsumer>(); // Default diag consumer outputs to cout

	setupDiagOpts();
}

DiagnosticEngine::DiagnosticEngine(std::unique_ptr<DiagnosticConsumer> ncons,FlagsManager *fm): consumer_(std::move(ncons)), flagsManager_(fm)
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

	// Silence this diag if needed
	if (shouldSilence(sev))
	{
		// Create an empty diagnostic object by calling the default constructor
		Diagnostic diag;
		return diag;
	}

	// Return
	if (haveTooManyErrorsOccured() && (!hasReportedErrLimitExceededError_)) 
	{
		// Override the diagnostic with a "Max error count exceeded" Diagnostic.
		hasReportedErrLimitExceededError_ = true;
		return report(DiagID::diagengine_maxErrCountExceeded).addArg(errLimit_).freeze(); /* Freeze the diagnostic to prevent user modifications */
	}
	else	
	{
		// If we return the user requested diagnostic, update the counters accordingly, then return.
		updateInternalCounters(sev);
		Diagnostic rtr_diag(
			consumer_.get(),
			diagID,
			sev,
			str
		);
		return rtr_diag;
	}
}

void DiagnosticEngine::setConsumer(std::unique_ptr<DiagnosticConsumer> ncons)
{
	consumer_ = std::move(ncons);
}

DiagnosticConsumer* DiagnosticEngine::getConsumer()
{
	return consumer_.get();
}

void DiagnosticEngine::setFlagsManager(FlagsManager * fm)
{
	flagsManager_ = fm;
	if(fm)
		updateOptionsFromFlags();
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
		// Errors are no longer emitted if too many errors have occured and the "too many errors" fatal error has been emitted
		case DiagSeverity::ERROR:
			return haveTooManyErrorsOccured() && hasReportedErrLimitExceededError_;
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

bool DiagnosticEngine::haveTooManyErrorsOccured() const
{
	if(errLimit_)
		return numErrors_ >= errLimit_;
	return false;
}