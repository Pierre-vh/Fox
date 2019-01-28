//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See LICENSE.txt for license info.            
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
  assert(decl  && "Declaration cannot be null!");
  assert(decl->getRange() && "Declaration must have valid source location"
    "information to be inserted in the DeclContext");
  assert(!decl->isLocal() && "Can't add local declarations in a DeclContext!");
  // Insert in decls_
  data().decls.push_back(decl);
  if(NamedDecl* named = dyn_cast<NamedDecl>(decl)) {
    // Update the lookup map if it has been built
    Identifier id = named->getIdentifier();
    assert(id && "NameDecl with invalid identifier");
    if(data().lookupMap) data().lookupMap->insert({id, named});
  }
}

const LookupContext::DeclVec& LookupContext::getDecls() const {
  return data().decls;
}

const LookupContext::LookupMap& LookupContext::getLookupMap() {
  if(!data().lookupMap)
    buildLookupMap();
  assert(data().lookupMap && "buildLookupMap() did not build the "
    "LookupMap!");
  return *(data().lookupMap);
}

std::size_t LookupContext::numDecls() const {
  return data().decls.size();
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
  DeclContext(kind, parent), data_(DeclData::create(ctxt, this)) {
  assert(data_ && "LookupContext data not created");      
}

LookupContext::DeclData& LookupContext::data() {
  assert(data_ && "Data cannot be nullptr!");
  return (*data_);
}

const LookupContext::DeclData& LookupContext::data() const {
  assert(data_ && "Data cannot be nullptr!");
  return (*data_);
}

void LookupContext::buildLookupMap() {
  // Don't do it if it's already built
  if(data().lookupMap) return;
  data().lookupMap = std::make_unique<LookupMap>();

  // Iterate over decls_ and insert them in the lookup map
  for(Decl* decl : data().decls) {
    if(NamedDecl* named = dyn_cast<NamedDecl>(decl)) {
      Identifier id = named->getIdentifier();
      data().lookupMap->insert({id, named});
    }
  }
}

//----------------------------------------------------------------------------//
// DeclData
//----------------------------------------------------------------------------//

LookupContext::DeclData* 
LookupContext::DeclData::create(ASTContext& ctxt, LookupContext* lc) {
  DeclData* inst = new(ctxt) DeclData(lc);
  // Important: register the cleanup, since this object isn't trivially
  // destructible.
  ctxt.addCleanup([inst](){inst->~DeclData();});
  return inst;
}

void* LookupContext::DeclData::operator new(std::size_t sz, ASTContext& ctxt, 
  std::uint8_t align) {
  void* rawMem = ctxt.allocate(sz, align);
  assert(rawMem);
  return rawMem;
}
