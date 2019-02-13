//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : DeclContext.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/DeclContext.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/SourceLoc.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// DeclContext
//----------------------------------------------------------------------------//

DeclContext::DeclContext(DeclContextKind kind, 
  DeclContext* parent): parentAndKind_(parent, toInt(kind)) {
  assert((parent || isa<UnitDecl>(this)) && "Every DeclContexts except "
    "UnitDecls must have a parent!");
}

DeclContextKind DeclContext::getDeclContextKind() const {
  return static_cast<DeclContextKind>(parentAndKind_.getInt());
}

ASTContext& DeclContext::getASTContext() const {
  if(const UnitDecl* unit = dyn_cast<UnitDecl>(this))
    return unit->getASTContext();
  return getParentDeclCtxt()->getASTContext();
}

bool DeclContext::hasParentDeclCtxt() const {
  return parentAndKind_.getPointer() != nullptr;
}

DeclContext* DeclContext::getParentDeclCtxt() const {
  return parentAndKind_.getPointer();
}

bool DeclContext::classof(const Decl* decl) {
  #define DECL_CTXT(ID, PARENT) case DeclKind::ID:
  switch(decl->getKind()) {
    #include "Fox/AST/DeclNodes.def"
      return true;
    default: 
      return false;
  }
}

//----------------------------------------------------------------------------//
// LookupContext
//----------------------------------------------------------------------------//

void LookupContext::addDecl(Decl* decl) {
  // Run some checks.
  assert(decl && 
    "Declaration cannot be null!");
  assert(decl->getRange() && "Declaration must have valid source location"
    "information to be inserted in the DeclContext");
  assert(!decl->isLocal() && 
    "Can't add local declarations in a DeclContext!");

  // Insert the decl_ in the linked list of decls
  if (firstDecl_) {
    assert(lastDecl_ && "firstDecl_ is not null but lastDecl_ is");
    lastDecl_ = lastDecl_->nextDecl_ = decl;
  }
  else {
    assert(!lastDecl_ && "firstDecl_ is null but lastDecl_ isn't");
    firstDecl_ = lastDecl_ = decl;
  }
  
  if(NamedDecl* named = dyn_cast<NamedDecl>(decl)) {
    // Update the lookup map if we have one.
    Identifier id = named->getIdentifier();
    assert(id && "NameDecl with invalid identifier");
    if(lookupMap_) 
      lookupMap_->insert({id, named});
  }
}

DeclRange LookupContext::getDecls() const {
  return DeclRange(firstDecl_, nullptr);
}

Decl* LookupContext::getFirstDecl() const {
  return firstDecl_;
}

Decl* LookupContext::getLastDecl() const {
  return lastDecl_;
}

const LookupContext::LookupMap& LookupContext::getLookupMap() {
  if(!lookupMap_)
    buildLookupMap();
  assert(lookupMap_ && "buildLookupMap() did not build the "
    "LookupMap!");
  return *lookupMap_;
}

bool LookupContext::classof(const Decl* decl) {
  #define LOOKUP_CTXT(ID, PARENT) case DeclKind::ID:
  switch(decl->getKind()) {
    #include "Fox/AST/DeclNodes.def"
      return true;
    default: 
      return false;
  }
}

bool LookupContext::classof(const DeclContext* dc) {
  #define LOOKUP_CTXT(ID, PARENT) case DeclContextKind::ID:
  switch(dc->getDeclContextKind()) {
    #include "Fox/AST/DeclNodes.def"
      return true;
    default: 
      return false;
  }
}

LookupContext::LookupContext(ASTContext& ctxt, DeclContextKind kind, 
                             DeclContext* parent):
  DeclContext(kind, parent), ctxt_(ctxt) {}

void LookupContext::buildLookupMap() {
  // Don't do it if it's already built
  if(lookupMap_) return;

  // Allocate the LookupMap
  void* mem = ctxt_.allocate(sizeof(LookupMap), alignof(LookupMap));
  lookupMap_ = new(mem) LookupMap();

  // Iterate over decls_ and insert them in the lookup map
  for(Decl* decl : getDecls()) {
    if(NamedDecl* named = dyn_cast<NamedDecl>(decl)) {
      Identifier id = named->getIdentifier();
      lookupMap_->insert({id, named});
    }
  }
}

//----------------------------------------------------------------------------//
// DeclIterator
//----------------------------------------------------------------------------//

DeclIterator::DeclIterator(Decl* cur) : cur_(cur) {}

DeclIterator& DeclIterator::operator++() {
  assert(cur_ && "incrementing past the end");
  cur_ = cur_->nextDecl_;
  return *this;
}

DeclIterator fox::DeclIterator::operator++(int) {
  DeclIterator old = *this;
  ++(*this);
  return old;
}

Decl* DeclIterator::operator*() const {
  return cur_;
}

Decl* DeclIterator::operator->() const {
  return cur_;
}

bool fox::operator==(DeclIterator lhs, DeclIterator rhs) {
  return lhs.cur_ == rhs.cur_;
}

bool fox::operator!=(DeclIterator lhs, DeclIterator rhs) {
  return lhs.cur_ != rhs.cur_;
}

//----------------------------------------------------------------------------//
// DeclRange
//----------------------------------------------------------------------------//

DeclRange::DeclRange(DeclIterator beg, DeclIterator end) 
  : beg_(beg), end_(end) {}

DeclIterator DeclRange::begin() const {
  return beg_;
}

DeclIterator DeclRange::end() const {
  return end_;
}

bool DeclRange::isEmpty() const {
  return beg_ == end_;
}
