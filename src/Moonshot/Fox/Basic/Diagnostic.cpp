////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Diagnostic.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Diagnostic.hpp"
#include "IDiagConsumer.hpp"

#include <iostream>
#include <cassert>

using namespace Moonshot;

Diagnostic::Diagnostic(IDiagConsumer * cons, const DiagID & dID, const DiagSeverity & dSev, const std::string& dStr) :
	consumer_(cons), diagID_(dID), diagSeverity_(dSev), diagStr_(dStr)
{

}

Diagnostic::Diagnostic(Diagnostic &other)
{
	consumer_		= other.consumer_;
	diagID_			= other.diagID_;
	diagStr_		= other.diagStr_;
	diagSeverity_	= other.diagSeverity_;
	isActive_		= other.isActive_;
	isFrozen_		= other.isFrozen_;
	// Kill the other diagnostic!
	other.kill();
}

Diagnostic Diagnostic::createDummyDiagnosticObject()
{
	Diagnostic diag;
	return diag;
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
		isActive_ = false;
		consumer_ = 0;
		diagStr_.clear();
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