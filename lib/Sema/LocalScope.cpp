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

bool LocalScope::add(NamedDecl* decl) {
  // In this method, we want to add without inserting
  Identifier id = decl->getIdentifier();
  assert(id && "decl must have a valid Identifier!");
  assert(decl->isLocal() && "decl must be local!");
  // Insert returns a pair whose second element is true if the
  // insertion occured.
  return ((decls_.insert({ id, decl })).second);
}

bool LocalScope::forceAdd(NamedDecl* decl) {
  Identifier id = decl->getIdentifier();
  assert(id && "decl must have a valid Identifier!");
  assert(decl->isLocal() && "decl must be local!");
  
  auto lb = decls_.lower_bound(id);
  if((lb != decls_.end()) && !(decls_.key_comp()(id, lb->first))) {
    // A decl with this name already exists in this map, overwrite it
    lb->second = decl;
    // Identifiers should strictly match
    assert(lb->first == lb->second->getIdentifier());
    // Overwrote something, return false
    return false;
  }
  else {
    // a decl with this name did not exist. Insert using the hint
    decls_.insert(lb, {decl->getIdentifier(), decl});
    // Didn't overwrite anything, return true
    return true;
  }
}

LocalScope::MapTy& LocalScope::getDeclsMap() {
  return decls_;
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
