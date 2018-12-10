//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : DeclContext.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/DeclContext.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

DeclContext::DeclContext(DeclContextKind kind, DeclContext* parent):
  parentAndKind_(parent, toInt(kind)) {

}

DeclContextKind DeclContext::getDeclContextKind() const {
  return static_cast<DeclContextKind>(parentAndKind_.getInt());
}

void DeclContext::addDecl(Decl* decl) {
  assert(decl  && "Declaration cannot be null!");
  NamedDecl* named = dyn_cast<NamedDecl>(decl);
  // At the time of writing this, every decl is derived from NamedDecl, so
  // this check never fails. It's only there as a guide to help me find out
  // where to implement unnamed decls support if I ever need it.
  if(!named) fox_unimplemented_feature("Unnamed Decls in DeclContext");
  Identifier name = named->getIdentifier();
  assert(name  
    && "Declaration must have a non-null Identifier to be "
       "recorded");
  decls_.insert({name, decl});
}

bool DeclContext::isLocalDeclContext() const {
  switch (getDeclContextKind()) {
    #define LOCAL_DECL_CTXT(ID, PARENT) case DeclContextKind::ID:
    #include "Fox/AST/DeclNodes.def"
      return true;
    default:
      return false;
  }
}

DeclContext::DeclsMapTy& DeclContext::getDeclsMap() {
  return decls_;
}

bool DeclContext::hasParent() const {
  return parentAndKind_.getPointer() != nullptr;
}

DeclContext* DeclContext::getParent() const {
  return parentAndKind_.getPointer();
}

void DeclContext::setParent(DeclContext* dr) {
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