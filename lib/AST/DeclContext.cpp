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

DeclContext::DeclContext(ASTContext& ctxt, DeclContextKind kind, 
                         DeclContext* parent):
  parentAndKind_(parent, toInt(kind)) {
  assert((parent || isa<UnitDecl>(this)) && "Every DeclContexts except "
    "UnitDecls must have a parent!");
  // Create the LookupMap
  void* mem = ctxt.allocate(sizeof(LookupMap), alignof(LookupMap));
  lookupMap_ = new(mem) LookupMap();
  // Add its cleanup
  ctxt.addDestructorCleanup(*lookupMap_);
  assert(lookupMap_);
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

bool DeclContext::isLocal() const {
  return getDeclContextKind() == DeclContextKind::FuncDecl
  ;
}

void DeclContext::addDecl(Decl* decl) {
  // Run some checks.
  assert(decl && 
    "Declaration cannot be null!");
  assert(decl->getRange() && "Declaration must have valid source location"
    "information to be inserted in the DeclContext");

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

DeclRange DeclContext::getDecls() const {
  return DeclRange(firstDecl_, nullptr);
}

Decl* DeclContext::getFirstDecl() const {
  return firstDecl_;
}

Decl* DeclContext::getLastDecl() const {
  return lastDecl_;
}

bool DeclContext::lookup(Identifier id, SourceLoc loc, 
                         ResultFoundCallback onFound) {
  assert(id && "Identifier is invalid");
  assert(onFound && "Callback is null, results will be discarded!");
  assert(lookupMap_ && "No lookupMap?");
  const LookupMap& map = *lookupMap_;
  // Search all decls with the identifier "id" in the multimap
  LookupMap::const_iterator beg, end;
  std::tie(beg, end) = map.equal_range(id);
  for(auto it = beg; it != end; ++it) {
    // Skip if the decl has been declared after the loc.
    if(loc && loc.comesBefore(it->second->getBegin()))
      continue;
    
    if(!onFound(it->second)) return false;
  }
  return true;
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
