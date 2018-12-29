//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : DiagnosticEngine.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/DiagnosticVerifier.hpp"
#include "Fox/Common/Source.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/StringManipulator.hpp"
#include <iostream>

using namespace fox;

//----------------------------------------------------------------------------//
// Diagnostic data
//----------------------------------------------------------------------------//

namespace {

  constexpr const char * const diagsStrs[] = {
    #define DIAG(SEVERITY,ID,TEXT) TEXT,
    #include "Fox/Common/Diags/All.def"
  };

  constexpr DiagSeverity diagsSevs[] = {
    #define DIAG(SEVERITY,ID,TEXT) DiagSeverity::SEVERITY,
    #include "Fox/Common/Diags/All.def"
  };

  constexpr unsigned numDiags = sizeof(diagsSevs);
}

//----------------------------------------------------------------------------//
// Diagnostic data
//----------------------------------------------------------------------------//

class Diagnostic::StaticAsserts {
  static_assert(numDiags < (1 << diagIdBits), "Too many diagnostics for the "
    "diagID_ bitfield. Increase diagIdBits!");
};

//----------------------------------------------------------------------------//
// Diagnostic Engine
//----------------------------------------------------------------------------//

DiagnosticEngine::DiagnosticEngine(SourceManager& sm, std::ostream& os):
  DiagnosticEngine(std::make_unique<StreamDiagConsumer>(sm, os)) {}

DiagnosticEngine::DiagnosticEngine(SourceManager& sm):
  DiagnosticEngine(sm, std::cout) {}

DiagnosticEngine::DiagnosticEngine(std::unique_ptr<DiagnosticConsumer> ncons):
  consumer_(std::move(ncons)) {
  errLimitReached_ = false;
  hasFatalErrorOccured_ = false;
  errorsAreFatal_ = false;
  ignoreAll_ = false;
  ignoreAllAfterFatalError_ = false;
  ignoreNotes_ = false;
  ignoreWarnings_ = false;
  warningsAreErrors_ = false;
}

Diagnostic DiagnosticEngine::report(DiagID diagID) {
  return report(diagID, SourceRange());
}

Diagnostic DiagnosticEngine::report(DiagID diagID, FileID file) {
  return report(diagID, SourceRange(SourceLoc(file))).setIsFileWide(true);
}

Diagnostic DiagnosticEngine::report(DiagID diagID, SourceRange range) {
  const auto idx = static_cast<std::underlying_type<DiagID>::type>(diagID);
  DiagSeverity sev = diagsSevs[idx];
  std::string str(diagsStrs[idx]);

  sev = changeSeverityIfNeeded(sev);

  return Diagnostic(this, diagID, sev, str, range);
}

Diagnostic DiagnosticEngine::report(DiagID diagID, SourceLoc loc) {
  return report(diagID, SourceRange(loc));
}

void DiagnosticEngine::enableVerifyMode(DiagnosticVerifier* dv) {
  assert(dv && "Can't enable verify mode with a null DiagnosticVerifier");
  verifier_ = dv;
}

bool DiagnosticEngine::isVerifyModeEnabled() const {
  return (bool)verifier_;
}

void DiagnosticEngine::disableVerifyMode() {
  verifier_ = nullptr;
}

void DiagnosticEngine::setConsumer(std::unique_ptr<DiagnosticConsumer> ncons) {
  consumer_ = std::move(ncons);
}

DiagnosticConsumer* DiagnosticEngine::getConsumer() {
  return consumer_.get();
}

const DiagnosticConsumer* DiagnosticEngine::getConsumer() const {
  return consumer_.get();
}

std::unique_ptr<DiagnosticConsumer> DiagnosticEngine::takeConsumer() {
  return std::move(consumer_);
}

bool DiagnosticEngine::hasFatalErrorOccured() const {
  return hasFatalErrorOccured_;
}

std::uint16_t DiagnosticEngine::getWarningsCount() const {
  return warnCount_;
}

std::uint16_t DiagnosticEngine::getErrorsCount() const {
  return errorCount_;
}

std::uint16_t DiagnosticEngine::getErrorLimit() const {
  return errLimit_;
}

void DiagnosticEngine::setErrorLimit(std::uint16_t mErr) {
  errLimit_ = mErr;
}

bool DiagnosticEngine::getWarningsAreErrors() const {
  return warningsAreErrors_;
}

void DiagnosticEngine::setWarningsAreErrors(bool val) {
  warningsAreErrors_ = val;
}

bool DiagnosticEngine::getErrorsAreFatal() const {
  return errorsAreFatal_;
}

void DiagnosticEngine::setErrorsAreFatal(bool val) {
  errorsAreFatal_ = val;
}

bool DiagnosticEngine::getIgnoreWarnings() const {
  return ignoreWarnings_;
}

void DiagnosticEngine::setIgnoreWarnings(bool val) {
  ignoreWarnings_ = val;
}

bool DiagnosticEngine::getIgnoreNotes() const {
  return ignoreNotes_;
}

void DiagnosticEngine::setIgnoreNotes(bool val) {
  ignoreNotes_ = val;
}

bool DiagnosticEngine::getIgnoreAllAfterFatal() const {
  return ignoreAllAfterFatalError_;
}

void DiagnosticEngine::setIgnoreAllAfterFatal(bool val) {
  ignoreAllAfterFatalError_ = val;
}

bool DiagnosticEngine::getIgnoreAll() const {
  return ignoreAll_;
}

void DiagnosticEngine::setIgnoreAll(bool val) {
  ignoreAll_ = val;
}

void DiagnosticEngine::handleDiagnostic(Diagnostic& diag) {
  if (diag.getSeverity() == DiagSeverity::Ignore)
    return;

  assert(consumer_ && "No valid consumer");

  bool canConsume = true;

  // Do verification if needed
  if(verifier_)
    canConsume = verifier_->verify(diag);

  // Let the consumer consume the diag if he can.
  if(canConsume)
    consumer_->consume(diag);

  // Update the internal state
  updateInternalCounters(diag.getSeverity());

  // Now, check if we must emit a "too many errors" error.
  if ((errLimit_ != 0) && (errorCount_ >= errLimit_)) {
    // If we should emit one, check if we haven't emitted one already.
    if (!errLimitReached_) {
      // Important : set this to true to avoid infinite recursion.
      errLimitReached_ = true;
      report(DiagID::diagengine_maxErrCountExceeded).addArg(errorCount_).emit();
      setIgnoreAll(true);
    }
  }
}

DiagSeverity DiagnosticEngine::changeSeverityIfNeeded(DiagSeverity ds) const {
  using Sev = DiagSeverity;

  if (getIgnoreAll())
    return Sev::Ignore;

  if (getIgnoreAllAfterFatal() && hasFatalErrorOccured())
    return Sev::Ignore;

  switch (ds) {
    // Ignored diags don't change
    case Sev::Ignore:
      return Sev::Ignore;
    // Notes are silenced if the corresponding option is set
    case Sev::Note:
      return getIgnoreNotes() ? Sev::Ignore : Sev::Note;
    // If Warnings must be silent, the warning is ignored.
    // Else, if the warnings are considered errors,
    // it is promoted to an error. If not, it stays a warning.
    case Sev::Warning:
      if (getIgnoreWarnings())
        return Sev::Ignore;
      return getWarningsAreErrors() ? Sev::Error : Sev::Warning;
    // Errors are Ignored if too many of them have occured.
    // Else, it stays an error except if errors should be considered
    // Fatal.
    case Sev::Error:
      return getErrorsAreFatal() ? Sev::Fatal : Sev::Error;
    // Fatal diags don't change
    case Sev::Fatal:
      return ds;
    default:
      fox_unreachable("unknown severity");
  }
}

void DiagnosticEngine::updateInternalCounters(DiagSeverity ds) {
  switch (ds) {
    case DiagSeverity::Warning:
      warnCount_++;
      break;
    case DiagSeverity::Error:
      errorCount_++;
      break;
    case DiagSeverity::Fatal:
      hasFatalErrorOccured_ = true;
      break;
  }
}

//----------------------------------------------------------------------------//
// Diagnostic
//----------------------------------------------------------------------------//

Diagnostic::Diagnostic(DiagnosticEngine* engine, DiagID dID,
  DiagSeverity dSev, const std::string& dStr, const SourceRange& range) :
  engine_(engine), diagID_(dID), diagSeverity_(dSev), diagStr_(dStr),
  range_(range) {
  assert(engine && "Engine cannot be null!");
  initBitFields();
}

Diagnostic::Diagnostic(Diagnostic &other) {
  *this = other;
  other.kill();
}

Diagnostic::Diagnostic(Diagnostic&& other) {
  *this = other;
  other.kill();
}

Diagnostic::~Diagnostic() {
  emit();
}

void Diagnostic::emit() {
  if (isActive()) {
    assert(engine_
      && "Attempting to emit without a DiagnosticEngine set!");
    engine_->handleDiagnostic(*this);
    kill(); // kill this diag once it's consumed.
  }
}

DiagID Diagnostic::getID() const {
  return diagID_;
}

std::string Diagnostic::getStr() const {
  return diagStr_;
}

DiagSeverity Diagnostic::getSeverity() const {
  return diagSeverity_;
}

FileID Diagnostic::getFileID() const {
  return range_.getFileID();
}

SourceRange Diagnostic::getRange() const {
  return range_;
}

Diagnostic& Diagnostic::setRange(SourceRange range) {
  range_ = range;
  return *this;
}

bool Diagnostic::hasRange() const {
  return range_.isValid();
}

SourceRange Diagnostic::getExtraRange() const {
  return extraRange_;
}

Diagnostic& Diagnostic::setExtraRange(SourceRange range) {
  extraRange_ = range;
  return *this;
}

bool Diagnostic::hasExtraRange() const {
  return extraRange_.isValid();
}

Diagnostic& Diagnostic::setIsFileWide(bool fileWide) {
  fileWide_ = fileWide;
  return *this;
}

bool Diagnostic::isFileWide() const {
  return fileWide_;
}

bool Diagnostic::isActive() const {
  return (bool)engine_;
}

Diagnostic& Diagnostic::replacePlaceholder(const std::string& replacement) {

  // This method can be quite expensive, so, as an optimization,
  // don't do it if the diagnostic isn't active.
  if (!isActive()) return *this;

  auto index = curPHIndex_;

  assert(index < (1 << placeholderIndexBits) && 
    "Trying to replace too many placeholders!");

  std::string targetPH = "%" + std::to_string((int)index);
  std::size_t n = 0;
  while ((n = diagStr_.find(targetPH, n)) != std::string::npos) {
    diagStr_.replace(n, targetPH.size(), replacement);
    n += replacement.size();
  }
  ++curPHIndex_;
  return *this;
}

Diagnostic& Diagnostic::replacePlaceholder(FoxChar replacement) {
  return replacePlaceholder(StringManipulator::charToStr(replacement));
}

void Diagnostic::kill() {
  if (isActive()) {
    // Clear all variables
    engine_ = nullptr;
    diagStr_.clear();
    diagSeverity_ = DiagSeverity::Ignore;
  }
}

Diagnostic::operator bool() const {
  return isActive();
}

void Diagnostic::initBitFields() {
  curPHIndex_ = 0;
  fileWide_ = false;
}

//----------------------------------------------------------------------------//
// DiagnSeverity helpers
//----------------------------------------------------------------------------//

std::string fox::toString(DiagSeverity sev, bool allCaps) {
  using DS = DiagSeverity;
  switch (sev) {
    case DS::Ignore:
      return allCaps ? "IGNORE" : "Ignore";
    case DS::Note:
      return allCaps ? "NOTE" : "Note";
    case DS::Warning:
      return allCaps ? "WARNING" : "Warning";
    case DS::Error:
      return allCaps ? "ERROR" : "Error";
    case DS::Fatal:
      return allCaps ? "FATAL" : "Fatal";
    default:
      fox_unreachable("all cases handled");
  }
}

std::ostream& fox::operator<<(std::ostream& os, DiagSeverity sev) {
  // Print the output in all caps as this operator is mostly used for 
  // debugging purposes and won't care about user friendliness.
  os << toString(sev, /* all caps */ true);
  return os;
}