//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : DeclContext.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/DeclContext.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/Source.hpp"

using namespace fox;

DeclContext::DeclContext(DeclContextKind kind, DeclContext* parent):
  parentAndKind_(parent, toInt(kind)) {

}

DeclContextKind DeclContext::getDeclContextKind() const {
  return static_cast<DeclContextKind>(parentAndKind_.getInt());
}

void DeclContext::addDecl(Decl* decl) {
  assert(decl  && "Declaration cannot be null!");
  assert(decl->getRange() && "Declaration must have valid source location"
    "information to be inserted in the DeclContext");
  assert(!decl->isLocal() && "Can't add local declarations in a DeclContext!");
  // Insert in decls_
  decls_.push_back(decl);
  if(NamedDecl* named = dyn_cast<NamedDecl>(decl)) {
    // Update the lookup map if it has been built
    Identifier id = named->getIdentifier();
    assert(id && "NameDecl with invalid identifier");
    if(lookup_) lookup_->insert({id, named});
  }
}

bool DeclContext::hasParentDeclCtxt() const {
  return parentAndKind_.getPointer() != nullptr;
}

DeclContext* DeclContext::getParentDeclCtxt() const {
  return parentAndKind_.getPointer();
}

const DeclContext::DeclVec& DeclContext::getDecls() const {
  return decls_;
}

const DeclContext::LookupMap& DeclContext::getLookupMap() {
  if(!lookup_)
    buildLookupMap();
  assert(lookup_ && "buildLookupMap() did not build the "
    "LookupMap!");
  return *lookup_;
}

void DeclContext::setParentDeclCtxt(DeclContext* dr) {
  parentAndKind_.setPointer(dr);
}

std::size_t DeclContext::numDecls() const {
  return decls_.size();
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

void DeclContext::buildLookupMap() {
  // Don't do it if it's already built
  if(lookup_) return;
  lookup_ = std::make_unique<LookupMap>();

  // Iterate over decls_ and insert them in the lookup map
  for(Decl* decl : decls_) {
    if(NamedDecl* named = dyn_cast<NamedDecl>(decl)) {
      Identifier id = named->getIdentifier();
      lookup_->insert({id, named});
    }
  }
}