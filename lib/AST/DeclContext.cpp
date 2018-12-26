//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : DeclContext.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/DeclContext.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/Source.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// DeclContext
//----------------------------------------------------------------------------//

DeclContext::DeclContext(ASTContext& ctxt, DeclContextKind kind, 
  DeclContext* parent): parentAndKind_(parent, toInt(kind)),
  data_(DeclData::create(ctxt, this)) {
  assert(data_ && "DeclContext data not created");  
  assert((parent || isa<UnitDecl>(this)) && "Every DeclContexts except "
    "UnitDecls must have a parent!");
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
  data().decls.push_back(decl);
  if(NamedDecl* named = dyn_cast<NamedDecl>(decl)) {
    // Update the lookup map if it has been built
    Identifier id = named->getIdentifier();
    assert(id && "NameDecl with invalid identifier");
    if(data().lookupMap) data().lookupMap->insert({id, named});
  }
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

const DeclContext::DeclVec& DeclContext::getDecls() const {
  return data().decls;
}

const DeclContext::LookupMap& DeclContext::getLookupMap() {
  if(!data().lookupMap)
    buildLookupMap();
  assert(data().lookupMap && "buildLookupMap() did not build the "
    "LookupMap!");
  return *(data().lookupMap);
}

std::size_t DeclContext::numDecls() const {
  return data().decls.size();
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

DeclContext::DeclData& DeclContext::data() {
  assert(data_ && "Data cannot be nullptr!");
  return (*data_);
}

const DeclContext::DeclData& DeclContext::data() const {
  assert(data_ && "Data cannot be nullptr!");
  return (*data_);
}

void DeclContext::buildLookupMap() {
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

DeclContext::DeclData* 
DeclContext::DeclData::create(ASTContext& ctxt, DeclContext* dc) {
  DeclData* inst = new(ctxt) DeclData(dc);
  // Important: register the cleanup, since this object isn't trivially
  // destructible.
  ctxt.addCleanup([inst](){inst->~DeclData();});
  return inst;
}

void* DeclContext::DeclData::operator new(std::size_t sz, ASTContext& ctxt, 
  std::uint8_t align) {
  void* rawMem = ctxt.allocate(sz, align);
  assert(rawMem);
  return rawMem;
}
