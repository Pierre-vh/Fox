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

void LocalScope::add(NamedDecl* decl) {
  Identifier id = decl->getIdentifier();
  assert(id && "decl must have a valid Identifier!");
  decls_.insert({ id, decl });
}

LocalScope::MapTy& LocalScope::getMap() {
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
