////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : DeclRecorder.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////


#include "DeclRecorder.hpp"

#include "Decl.hpp"
#include <cassert>

using namespace fox;

DeclRecorder::DeclRecorder(DeclRecorder * parent) : parent_(parent)
{

}

void DeclRecorder::recordDecl(NamedDecl * decl)
{
	assert(decl	&& "Declaration cannot be null!");
	IdentifierInfo* name = decl->getIdentifier();
	assert(name	&& "Declaration must have a valid name (IdentifierInfo*) to be recorded!");
	namedDecls_.insert(std::make_pair(name, decl));
}

LookupResult DeclRecorder::restrictedLookup(IdentifierInfo * id) const
{
	auto it_range = namedDecls_.equal_range(id);
	LookupResult lr;
	for (auto it = it_range.first; it != it_range.second; it++)
		lr.addResult(it->second);
	return lr;
}

LookupResult DeclRecorder::fullLookup(IdentifierInfo * id) const
{
	auto this_lr = restrictedLookup(id);

	if (parent_)
	{
		auto parent_lr = parent_->fullLookup(id);
		this_lr.merge(parent_lr);
	}
		
	return this_lr;
}

bool DeclRecorder::hasParentDeclRecorder() const
{
	return parent_;
}

DeclRecorder * DeclRecorder::getParentDeclRecorder()
{
	return parent_;
}

void DeclRecorder::setParentDeclRecorder(DeclRecorder * dr)
{
	assert(dr && "Can't a null parent! Use resetParent() for that!");
	parent_ = dr;
}

void DeclRecorder::resetParentDeclRecorder()
{
	parent_ = nullptr;
}

std::size_t DeclRecorder::getNumberOfRecordedDecls() const
{
	return namedDecls_.size();
}

DeclRecorder::NamedDeclsMapIter DeclRecorder::recordedDecls_begin()
{
	return namedDecls_.begin();
}

DeclRecorder::NamedDeclsMapIter DeclRecorder::recordedDecls_end()
{
	return namedDecls_.end();
}

DeclRecorder::NamedDeclsMapConstIter DeclRecorder::recordedDecls_begin() const
{
	return namedDecls_.begin();
}

DeclRecorder::NamedDeclsMapConstIter DeclRecorder::recordedDecls_end() const
{
	return namedDecls_.end();
}

LookupResult::LookupResult()
{
	containsFuncDecl_ = false;
	containsVarDecl_ = false;
}

// LookupResult
bool LookupResult::isEmpty() const
{
	return !results_.size();
}

bool LookupResult::isUnique() const
{
	return (results_.size() == 1);
}

NamedDecl * LookupResult::getResultIfUnique() const
{
	return (results_.size() == 1) ? results_[0] : nullptr;
}

bool LookupResult::containsFunctionDecls() const
{
	return containsFuncDecl_;
}

bool LookupResult::containsVarDecl() const
{
	return containsVarDecl_;
}

bool LookupResult::onlyContainsFunctionDecls() const
{
	for (auto it = results_.begin(); it != results_.end(); it++)
	{
		// Return false if one of the results can't be dynamic_cast to a FunctionDecl*
		if (!dynamic_cast<FunctionDecl*>(*it))
			return false;
	}
	// Only returns true if there's at least one result.
	return (results_.size() != 0);
}

LookupResult::operator bool() const
{
	return !isEmpty();
}

void LookupResult::addResult(NamedDecl * decl)
{
	if (results_.size())
		assert((results_.back()->getIdentifier() == decl->getIdentifier()) && "A LookupResult can only contain NamedDecls that share the same identifier.");

	if (dynamic_cast<FunctionDecl*>(decl))
		containsFuncDecl_ = true;
	else if (dynamic_cast<VarDecl*>(decl))
		containsVarDecl_ = true;

	results_.push_back(decl);
}

void LookupResult::clear()
{
	containsFuncDecl_ = false;
	containsVarDecl_ = false;
	results_.clear();
}

void LookupResult::merge(LookupResult & target)
{
	if(target.results_.size() != 0)
		results_.insert(results_.end(), target.results_.begin(), target.results_.end());
	
	// update flags appropriately.
	containsFuncDecl_	= containsFuncDecl_ || target.containsFuncDecl_;
	containsVarDecl_	= containsVarDecl_ || target.containsVarDecl_;

	// Clear target.
	target.clear();
}


