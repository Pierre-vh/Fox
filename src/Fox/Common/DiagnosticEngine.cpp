////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : DiagnosticEngine.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "DiagnosticEngine.hpp"
#include "Diagnostic.hpp"
#include "DiagnosticConsumers.hpp"
#include "Fox/Common/Utils.hpp"
#include <cassert>

using namespace fox;

static const char* diagsStrs[] = {
	#define DIAG(SEVERITY,ID,TEXT) TEXT,
		#include "Diags/DiagsAll.def"
};

static const DiagSeverity diagsSevs[] = {
	#define DIAG(SEVERITY,ID,TEXT) DiagSeverity::SEVERITY,
		#include "Diags/DiagsAll.def"
};

DiagnosticEngine::DiagnosticEngine(SourceManager* sm) : DiagnosticEngine(std::make_unique<StreamDiagConsumer>(sm))
{

}

DiagnosticEngine::DiagnosticEngine(std::unique_ptr<DiagnosticConsumer> ncons): consumer_(std::move(ncons))
{
	resetAllOptions();
}

Diagnostic DiagnosticEngine::report(const DiagID& diagID)
{
	return report(diagID, SourceRange());
}

Diagnostic DiagnosticEngine::report(const DiagID& diagID, const SourceRange& range)
{
	// Gather diagnostic data
	const auto idx = enumAsInt(diagID);
	DiagSeverity sev = diagsSevs[idx];
	std::string str(diagsStrs[idx]);

	// Promote severity if needed
	sev = changeSeverityIfNeeded(sev);

	return Diagnostic(
				this,
				diagID,
				sev,
				str,
				range
			);;
}

Diagnostic DiagnosticEngine::report(const DiagID& diagID, const SourceLoc& loc)
{
	return report(diagID, SourceRange(loc));
}

void DiagnosticEngine::setConsumer(std::unique_ptr<DiagnosticConsumer> ncons)
{
	consumer_ = std::move(ncons);
}

DiagnosticConsumer* DiagnosticEngine::getConsumer()
{
	return consumer_.get();
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

unsigned int DiagnosticEngine::getWarningsCount() const
{
	return warnCount_;
}

unsigned int DiagnosticEngine::getErrorsCount() const
{
	return errorCount_;
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

void DiagnosticEngine::handleDiagnostic(Diagnostic& diag)
{
	updateInternalCounters(diag.getDiagSeverity());

	if (diag.getDiagSeverity() != DiagSeverity::IGNORE)
	{
		assert(consumer_ && "No valid consumer");
		consumer_->consume(diag);
	}

	// Now, check if we must emit a "too many errors" error.
	if (haveTooManyErrorsOccured())
	{
		if (!hasReportedErrLimitExceededError_)
		{
			hasReportedErrLimitExceededError_ = true;
			report(DiagID::diagengine_maxErrCountExceeded).addArg(errorCount_).emit();
			setSilenceAll(true);
		}
	}
}

DiagSeverity DiagnosticEngine::changeSeverityIfNeeded(const DiagSeverity& ds) const
{
	using Sev = DiagSeverity;

	if (getSilenceAll())
		return Sev::IGNORE;

	if (getSilenceAllAfterFatalErrors() && hasFatalErrorOccured())
		return Sev::IGNORE;

	switch (ds)
	{
		// Ignored diags don't change
		case Sev::IGNORE:
			return Sev::IGNORE;
		// Notes are silenced if the corresponding option is set
		case Sev::NOTE:
			return getSilenceNotes() ? Sev::IGNORE : Sev::NOTE;
		// If Warnings must be silent, the warning is ignored.
		// Else, if the warnings are considered errors,
		// it is promoted to an error. If not, it stays a warning.
		case Sev::WARNING:
			if (getSilenceWarnings())
				return Sev::IGNORE;
			return getWarningsAreErrors() ? Sev::ERROR : Sev::WARNING;
		// Errors are Ignored if too many of them have occured.
		// Else, it stays an error except if errors should be considered
		// Fatal.
		case Sev::ERROR:
			return getErrorsAreFatal() ? Sev::FATAL : Sev::ERROR;
		// Fatal diags don't change
		case Sev::FATAL:
			return ds;
		default:
			fox_unreachable("unknown severity");
	}
}

void DiagnosticEngine::updateInternalCounters(const DiagSeverity & ds)
{
	switch (ds)
	{
		case DiagSeverity::WARNING:
			warnCount_++;
			break;
		case DiagSeverity::ERROR:
			errorCount_++;
			break;
		case DiagSeverity::FATAL:
			hasFatalErrorOccured_ = true;
			break;
	}
}

bool DiagnosticEngine::haveTooManyErrorsOccured() const
{
	// Only check if errLimit != 0
	if(errLimit_)
		return errorCount_ >= errLimit_;
	return false;
}