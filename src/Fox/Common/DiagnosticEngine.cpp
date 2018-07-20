////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : DiagnosticEngine.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "DiagnosticEngine.hpp"
#include "Source.hpp"
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

DiagnosticEngine::DiagnosticEngine(SourceManager& sm) : DiagnosticEngine(std::make_unique<StreamDiagConsumer>(sm))
{

}

DiagnosticEngine::DiagnosticEngine(std::unique_ptr<DiagnosticConsumer> ncons): consumer_(std::move(ncons))
{
	errLimitReached_			= false;
	hasFatalErrorOccured_		= false;
	errorsAreFatal_				= false;
	silenceAll_					= false;
	silenceAllAfterFatalError_	= false;
	silenceNotes_				= false;
	silenceWarnings_			= false;
	warningsAreErrors_			= false;
}

Diagnostic DiagnosticEngine::report(DiagID diagID)
{
	return report(diagID, SourceRange());
}

Diagnostic DiagnosticEngine::report(DiagID diagID, const FileID & file)
{
	return report(diagID, SourceRange(SourceLoc(file))).setIsFileWide(true);
}

Diagnostic DiagnosticEngine::report(DiagID diagID, const SourceRange& range)
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

Diagnostic DiagnosticEngine::report(DiagID diagID, const SourceLoc& loc)
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

bool DiagnosticEngine::hasFatalErrorOccured() const
{
	return hasFatalErrorOccured_;
}

std::uint16_t DiagnosticEngine::getWarningsCount() const
{
	return warnCount_;
}

std::uint16_t DiagnosticEngine::getErrorsCount() const
{
	return errorCount_;
}

std::uint16_t DiagnosticEngine::getErrorLimit() const
{
	return errLimit_;
}

void DiagnosticEngine::setErrorLimit(std::uint16_t mErr)
{
	errLimit_ = mErr;
}

bool DiagnosticEngine::getWarningsAreErrors() const
{
	return warningsAreErrors_;
}

void DiagnosticEngine::setWarningsAreErrors(bool val)
{
	warningsAreErrors_ = val;
}

bool DiagnosticEngine::getErrorsAreFatal() const
{
	return errorsAreFatal_;
}

void DiagnosticEngine::setErrorsAreFatal(bool val)
{
	errorsAreFatal_ = val;
}

bool DiagnosticEngine::getSilenceWarnings() const
{
	return silenceWarnings_;
}

void DiagnosticEngine::setSilenceWarnings(bool val)
{
	silenceWarnings_ = val;
}

bool DiagnosticEngine::getSilenceNotes() const
{
	return silenceNotes_;
}

void DiagnosticEngine::setSilenceNotes(bool val)
{
	silenceNotes_ = val;
}

bool DiagnosticEngine::getSilenceAllAfterFatalErrors() const
{
	return silenceAllAfterFatalError_;
}

void DiagnosticEngine::setSilenceAllAfterFatalErrors(bool val)
{
	silenceAllAfterFatalError_ = val;
}

bool DiagnosticEngine::getSilenceAll() const
{
	return silenceAll_;
}

void DiagnosticEngine::setSilenceAll(bool val)
{
	silenceAll_ = val;
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
	if ((errLimit_ != 0) && (errorCount_ >= errLimit_))
	{
		// If we should emit one, check if we haven't emitted one already.
		if (!errLimitReached_)
		{
			// Important : set this to true to avoid infinite recursion.
			errLimitReached_ = true;
			report(DiagID::diagengine_maxErrCountExceeded).addArg(errorCount_).emit();
			setSilenceAll(true);
		}
	}
}

DiagSeverity DiagnosticEngine::changeSeverityIfNeeded(DiagSeverity ds) const
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

void DiagnosticEngine::updateInternalCounters(DiagSeverity ds)
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
