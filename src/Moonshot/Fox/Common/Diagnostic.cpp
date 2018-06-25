////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Diagnostic.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Diagnostic.hpp"
#include "DiagnosticConsumers.hpp"
#include <iostream>
#include <cassert>

using namespace Moonshot;

Diagnostic::Diagnostic(DiagnosticConsumer* cons, const DiagID& dID, const DiagSeverity& dSev, const std::string& dStr, const SourceRange& range) :
	consumer_(cons), diagID_(dID), diagSeverity_(dSev), diagStr_(dStr), range_(range)
{

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

Diagnostic Diagnostic::createDummyDiagnosticObject()
{
	return Diagnostic();
}

Diagnostic::~Diagnostic()
{
	emit();
}

void Diagnostic::emit()
{
	if (isActive_ && (diagSeverity_ != DiagSeverity::IGNORE))
	{
		assert(consumer_ && "No consumer available!");
		consumer_->consume(*this);
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

Diagnostic::Diagnostic()
{
	// Diag starts frozen & inactive (won't be modified or emitted)
	isActive_ = false;
	isFrozen_ = true;
	// Init all members to a default value
	diagID_ = DiagID::dummyDiag;
	consumer_ = nullptr;
	diagStr_ = "";
	diagSeverity_ = DiagSeverity::IGNORE;
}

Diagnostic& Diagnostic::replacePlaceholder(const std::string & replacement, const unsigned char & index)
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
		consumer_ = 0;
		diagStr_.clear();
		isFrozen_ = true;
		range_ = SourceRange();
		diagID_ = DiagID::dummyDiag;
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

bool Diagnostic::hasValidConsumer() const
{
	return (bool)consumer_;
}

Diagnostic::operator bool() const
{
	return isActive() && consumer_;
}