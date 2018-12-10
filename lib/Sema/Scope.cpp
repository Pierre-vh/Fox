//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Scope.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Sema/Scope.hpp"
#include "Fox/AST/Decl.hpp"

using namespace fox;

VarDecl* Scope::add(VarDecl* decl) {
  LookupResultTy result;
  Identifier id = decl->getIdentifier();
  lookup(id, result);
  // Decl already exists in this Scope.
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

void Scope::lookup(Identifier id, LookupResultTy& results) {
  Scope* cur = this;
  while (cur) {
    cur->lookupImpl(id, results);
    cur = cur->getParent();
  }
}

void Scope::lookupImpl(Identifier id, LookupResultTy& results) {
  auto it = decls_.find(id);
  if (it != decls_.end())
    results.push_back(it->second);
}

Scope* Scope::getParent() const {
  return parent_;
}

bool Scope::hasParent() const {
  return (bool)parent_;
}

void Scope::setParent(Scope* scope) {
  parent_ = scope;
}
