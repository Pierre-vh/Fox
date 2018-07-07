////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Diagnostic.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Diagnostic.hpp"
#include "DiagnosticEngine.hpp"
#include <cassert>

using namespace fox;

Diagnostic::Diagnostic(DiagnosticEngine* engine, DiagID dID, DiagSeverity dSev, const std::string& dStr, const SourceRange& range) :
	engine_(engine), diagID_(dID), diagSeverity_(dSev), diagStr_(dStr), range_(range)
{
	assert(engine && "Engine cannot be null!");

	// Default values
	isActive_ = true;
	isFrozen_ = false;
	curPHIndex_ = 0;
}

Diagnostic::Diagnostic(Diagnostic &other)
{
	*this = other;
	other.kill();
}

Diagnostic::Diagnostic(Diagnostic&& other)
{
	*this = other;
	other.kill();
}

Diagnostic::~Diagnostic()
{
	emit();
}

void Diagnostic::emit()
{
	if (isActive_)
	{
		assert(engine_ && "Attempting to emit without a DiagnosticEngine set!");
		engine_->handleDiagnostic(*this);
		kill(); // kill this diag once it's consumed.
	}
}

DiagID Diagnostic::getDiagID() const
{
	return diagID_;
}

std::string Diagnostic::getDiagStr() const
{
	return diagStr_;
}

DiagSeverity Diagnostic::getDiagSeverity() const
{
	return diagSeverity_;
}

SourceRange Diagnostic::getSourceRange() const
{
	return range_;
}

bool Diagnostic::hasValidSourceRange() const
{
	return range_.isValid();
}

bool Diagnostic::isActive() const
{
	return isActive_;
}

Diagnostic& Diagnostic::replacePlaceholder(const std::string & replacement, std::uint8_t index)
{
	if (!isActive_ || isFrozen_)
		return *this;

	std::string targetPH = "%" + std::to_string((int)index);
	std::size_t n = 0;
	while ((n = diagStr_.find(targetPH, n)) != std::string::npos)
	{
		diagStr_.replace(n, targetPH.size(), replacement);
		n += replacement.size();
	}
	return *this;
}

void Diagnostic::kill()
{
	if (isActive_)
	{
		// Clear all variables
		isActive_ = false;
		engine_ = nullptr;
		diagStr_.clear();
		isFrozen_ = true;
		diagSeverity_ = DiagSeverity::IGNORE;
	}
}
bool Diagnostic::isFrozen() const
{
	return isFrozen_;
}

Diagnostic& Diagnostic::freeze()
{
	isFrozen_ = true;
	return *this;
}

Diagnostic::operator bool() const
{
	return isActive();
}