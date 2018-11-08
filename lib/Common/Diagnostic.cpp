//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Diagnostic.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/Diagnostic.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include <cassert>

using namespace fox;

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
  if (active_) {
    assert(engine_
      && "Attempting to emit without a DiagnosticEngine set!");
    engine_->handleDiagnostic(*this);
    kill(); // kill this diag once it's consumed.
  }
}

void Diagnostic::ignore() {
  diagSeverity_ = DiagSeverity::IGNORE;
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
  if(!frozen_)
    fileWide_ = fileWide;
  return *this;
}

bool Diagnostic::isFileWide() const {
  return fileWide_;
}

bool Diagnostic::isActive() const {
  return active_;
}

Diagnostic& Diagnostic::replacePlaceholder(const std::string&
  replacement, std::uint8_t index) {
  if (!active_ || frozen_)
    return *this;

  std::string targetPH = "%" + std::to_string((int)index);
  std::size_t n = 0;
  while ((n = diagStr_.find(targetPH, n)) != std::string::npos) {
    diagStr_.replace(n, targetPH.size(), replacement);
    n += replacement.size();
  }
  return *this;
}

void Diagnostic::kill() {
  if (active_) {
    // Clear all variables
    active_ = false;
    engine_ = nullptr;
    diagStr_.clear();
    frozen_ = true;
    diagSeverity_ = DiagSeverity::IGNORE;
  }
}
bool Diagnostic::isFrozen() const {
  return frozen_;
}

Diagnostic& Diagnostic::freeze() {
  frozen_ = true;
  return *this;
}

Diagnostic::operator bool() const {
  return isActive();
}

void Diagnostic::initBitFields() {
  active_ = true;
  frozen_ = false;
  curPHIndex_ = 0;
  fileWide_ = false;
}
