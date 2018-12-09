//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : DeclContext.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/DeclContext.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

DeclContext::DeclContext(DeclContextKind kind, DeclContext* parent):
  parentAndKind_(parent, toInt(kind)) {

}

DeclContextKind DeclContext::getDeclContextKind() const {
  return static_cast<DeclContextKind>(parentAndKind_.getInt());
}

void DeclContext::recordDecl(NamedDecl* decl) {
  assert(decl  && "Declaration cannot be null!");
  Identifier name = decl->getIdentifier();
  assert(name  
    && "Declaration must have a non-null Identifier to be "
       "recorded");
  namedDecls_.insert({name, decl});
}

LookupResult DeclContext::restrictedLookup(Identifier id) const {
  auto it_range = namedDecls_.equal_range(id);
  LookupResult lr;
  for (auto it = it_range.first; it != it_range.second; it++)
    lr.addResult(it->second);
  return lr;
}

LookupResult DeclContext::fullLookup(Identifier id) const {
  auto this_lr = restrictedLookup(id);
  if (getParent()) {
    auto parent_lr = getParent()->fullLookup(id);
    this_lr.absorb(parent_lr);
  }
  return this_lr;
}

bool DeclContext::hasParent() const {
  return parentAndKind_.getPointer() != nullptr;
}

DeclContext* DeclContext::getParent() {
  return parentAndKind_.getPointer();
}

const DeclContext* DeclContext::getParent() const {
  return parentAndKind_.getPointer();
}

void DeclContext::setParent(DeclContext* dr) {
  parentAndKind_.setPointer(dr);
}

std::size_t DeclContext::getNumberOfRecordedDecls() const {
  return namedDecls_.size();
}

DeclContext::NamedDeclsMapIter DeclContext::recordedDecls_begin() {
  return namedDecls_.begin();
}

DeclContext::NamedDeclsMapIter DeclContext::recordedDecls_end() {
  return namedDecls_.end();
}

DeclContext::NamedDeclsMapConstIter DeclContext::recordedDecls_begin() const {
  return namedDecls_.begin();
}

DeclContext::NamedDeclsMapConstIter DeclContext::recordedDecls_end() const {
  return namedDecls_.end();
}

bool DeclContext::classof(const Decl* decl)
{
  #define DECL_CTXT(ID, PARENT) case DeclKind::ID:
  switch(decl->getKind()) {
    #include "Fox/AST/DeclNodes.def"
      return true;
    default: 
      return false;
  }
}

// LookupResult
LookupResult::LookupResult() {

}

std::size_t LookupResult::size() const {
  return results_.size();
}

LookupResult::ResultVecIter LookupResult::begin() {
  return results_.begin();
}

LookupResult::ResultVecConstIter LookupResult::begin() const {
  return results_.begin();
}

LookupResult::ResultVecIter LookupResult::end() {
  return results_.end();
}

LookupResult::ResultVecConstIter LookupResult::end() const {
  return results_.end();
}

LookupResult::operator bool() const {
  return (size() != 0);
}

void LookupResult::addResult(NamedDecl* decl) {
  if (results_.size())
    assert((results_.back()->getIdentifier() == decl->getIdentifier()) 
      && "A LookupResult can only contain NamedDecls that share the same identifier.");

  results_.push_back(decl);
}

void LookupResult::absorb(LookupResult& target) {
  if (target.results_.size() == 0)
    return;

  results_.insert(results_.end(), target.results_.begin(), target.results_.end());
  results_.clear();
}