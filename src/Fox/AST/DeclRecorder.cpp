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
#include "Fox/Common/Utils.hpp"

using namespace fox;

DeclRecorder::DeclRecorder(DeclRecorder * parent) : parent_(parent)
{

}

void DeclRecorder::recordDecl(NamedDecl* decl)
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
		this_lr.absorb(parent_lr);
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

void DeclRecorder::setParentDeclRecorder(DeclRecorder* dr)
{
	assert(dr && "Can't set a null parent! Use resetParent() for that!");
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

}

// LookupResult
bool LookupResult::isEmpty() const
{
	return (getSize() == 0);
}

bool LookupResult::isUnique() const
{
	return (getSize() == 1);
}

std::size_t LookupResult::getSize() const
{
	return results_.size();
}

NamedDecl* LookupResult::getResultIfUnique() const
{
	return isUnique() ? results_[0] : nullptr;
}

LookupResult::operator bool() const
{
	return !isEmpty();
}

void LookupResult::addResult(NamedDecl* decl)
{
	if (results_.size())
		assert((results_.back()->getIdentifier() == decl->getIdentifier()) 
			&& "A LookupResult can only contain NamedDecls that share the same identifier.");

	results_.push_back(decl);
}

void LookupResult::clear()
{
	results_.clear();
}

void LookupResult::absorb(LookupResult& target)
{
	if (target.results_.size() == 0)
		return;

	results_.insert(results_.end(), target.results_.begin(), target.results_.end());
	target.clear();
}


