//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : DiagnosticEngine.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/DiagnosticVerifier.hpp"
#include "Fox/Common/SourceManager.hpp"
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
  DiagnosticEngine(sm, std::make_unique<StreamDiagConsumer>(os)) {}

DiagnosticEngine::DiagnosticEngine(SourceManager& sm):
  DiagnosticEngine(sm, std::cout) {}

DiagnosticEngine::DiagnosticEngine(SourceManager& sm, 
                                   std::unique_ptr<DiagnosticConsumer> ncons):
  consumer_(std::move(ncons)), srcMgr_(sm) {
  hadFatalError_ = false;
  hadError_ = false;
  ignoreAll_ = false;
  ignoreAllAfterFatalError_ = false;
  ignoreNotes_ = false;
  ignoreWarnings_ = false;
  warningsAreErrors_ = false;
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

bool DiagnosticEngine::hadFatalError() const {
  return hadFatalError_;
}

bool DiagnosticEngine::hadAnyError() const {
  return hadError_ || hadFatalError_;
}

bool DiagnosticEngine::getWarningsAreErrors() const {
  return warningsAreErrors_;
}

void DiagnosticEngine::setWarningsAreErrors(bool val) {
  warningsAreErrors_ = val;
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
    consumer_->consume(srcMgr_, diag);

  // Update the internal state
  updateInternalState(diag.getSeverity());
}

DiagSeverity DiagnosticEngine::changeSeverityIfNeeded(DiagSeverity ds) const {
  using Sev = DiagSeverity;

  if (getIgnoreAll())
    return Sev::Ignore;

  if (getIgnoreAllAfterFatal() && hadFatalError())
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
    // Errors don't change
    case Sev::Error:
      return ds;
    // Fatal diags don't change
    case Sev::Fatal:
      return ds;
    default:
      fox_unreachable("unknown severity");
  }
}

void DiagnosticEngine::updateInternalState(DiagSeverity ds) {
  switch (ds) {
    case DiagSeverity::Warning:
      // no-op
      break;
    case DiagSeverity::Error:
      hadError_ = true;
      break;
    case DiagSeverity::Fatal:
      hadFatalError_ = true;
      break;
    default:
      // no-op
      break;
  }
}

//----------------------------------------------------------------------------//
// Diagnostic
//----------------------------------------------------------------------------//

Diagnostic::Diagnostic(DiagnosticEngine* engine, DiagID dID,
  DiagSeverity dSev, string_view dStr, SourceRange range) :
  engine_(engine), diagID_(dID), diagSeverity_(dSev), diagStr_(dStr.to_string()),
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
  return (bool)range_ && !isFileWide();
}

bool Diagnostic::hasAnyLocInfo() const {
  return (bool)range_;
}

SourceRange Diagnostic::getExtraRange() const {
  return extraRange_;
}

Diagnostic& Diagnostic::setExtraRange(SourceRange range) {
  assert(hasRange() && "setting the extra range without a "
    "primary range");
  extraRange_ = range;
  return *this;
}

bool Diagnostic::hasExtraRange() const {
  return (bool)extraRange_;
}

Diagnostic& Diagnostic::setIsFileWide(bool fileWide) {
  assert(range_ && "a diagnostic cannot be file-wide "
    "if it doesn't have a valid FileID!");
  fileWide_ = fileWide;
  return *this;
}

bool Diagnostic::isFileWide() const {
  return fileWide_;
}

bool Diagnostic::isActive() const {
  return (bool)engine_;
}

Diagnostic& Diagnostic::replacePlaceholder(string_view replacement) {

  // This method can be quite expensive, so, as an optimization,
  // don't do it if the diagnostic isn't active.
  if (!isActive()) return *this;

  auto index = curPHIndex_;

  assert(index < (1 << placeholderIndexBits) && 
    "Trying to replace too many placeholders!");

  std::string targetPH = "%" + std::to_string((int)index);
  std::size_t n = 0;
  while ((n = diagStr_.find(targetPH, n)) != std::string::npos) {
    diagStr_.replace(n, targetPH.size(), replacement.to_string());
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

std::string fox::toString(DiagSeverity sev) {
  using DS = DiagSeverity;
  switch (sev) {
    case DS::Ignore:
      return "ignore";
    case DS::Note:
      return "note";
    case DS::Warning:
      return "warning";
    case DS::Error:
      return "error";
    case DS::Fatal:
      return "fatal";
    default:
      fox_unreachable("all cases handled");
  }
}

std::ostream& fox::operator<<(std::ostream& os, DiagSeverity sev) {
  os << toString(sev);
  return os;
}