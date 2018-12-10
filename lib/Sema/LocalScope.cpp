//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : LocalScope.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Sema/LocalScope.hpp"
#include "Fox/AST/Decl.hpp"

using namespace fox;

LocalScope::LocalScope(LocalScope* parent) : parent_(parent) {}

NamedDecl* LocalScope::add(NamedDecl* decl) {
  LookupResultTy result;
  Identifier id = decl->getIdentifier();
  lookup(id, result);
  // Decl already exists in this LocalScope.
  if(result.size()) {
    // In theory, only one result should have been found since
    // we never add duplicates
    assert(result.size() == 1);
    return result.back();
  }
  // Decl doesn't exist, add it in this scope.
  auto res = decls_.insert({ id, decl });
  assert(res.second && "No insertion occured?");
  // Success
  return nullptr;
}

void LocalScope::lookup(Identifier id, LookupResultTy& results) {
  LocalScope* cur = this;
  while (cur) {
    cur->lookupImpl(id, results);
    cur = cur->getParent();
  }
}

void LocalScope::lookupImpl(Identifier id, LookupResultTy& results) {
  auto it = decls_.find(id);
  if (it != decls_.end())
    results.push_back(it->second);
}

LocalScope* LocalScope::getParent() const {
  return parent_;
}

bool LocalScope::hasParent() const {
  return (bool)parent_;
}

void LocalScope::setParent(LocalScope* scope) {
  parent_ = scope;
}
