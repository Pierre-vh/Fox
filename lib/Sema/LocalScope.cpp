//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See LICENSE.txt for license info.            
// File : LocalScope.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Sema/LocalScope.hpp"
#include "Fox/AST/Decl.hpp"

using namespace fox;

LocalScope::LocalScope(Parent parent) : parent_(parent) {}

bool LocalScope::insert(NamedDecl* decl) {
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

LocalScope* LocalScope::getParentIfLocalScope() const {
  return parent_.dyn_cast<LocalScope*>();;
}

FuncDecl* LocalScope::getFuncDecl() const {
  if(FuncDecl* fn = parent_.dyn_cast<FuncDecl*>())
    return fn;
  return parent_.get<LocalScope*>()->getFuncDecl();
}

LocalScope::Map& LocalScope::getDeclsMap() {
  return decls_;
}

bool LocalScope::hasParent() const {
  return (bool)parent_;
}

LocalScope::Parent LocalScope::getParent() const {
  return parent_;
}

void LocalScope::setParent(Parent parent) {
  parent_ = parent;
}
