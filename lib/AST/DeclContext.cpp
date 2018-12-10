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

void DeclContext::recordDecl(NamedDecl* decl) {
  assert(decl  && "Declaration cannot be null!");
  Identifier name = decl->getIdentifier();
  assert(name  
    && "Declaration must have a non-null Identifier to be "
       "recorded");
  namedDecls_.insert({name, decl});
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
  return namedDecls_.size();
}

DeclContext::DeclMapIter DeclContext::decls_begin() {
  return namedDecls_.begin();
}

DeclContext::DeclMapIter DeclContext::decls_end() {
  return namedDecls_.end();
}

DeclContext::DeclMapConstIter DeclContext::decls_begin() const {
  return namedDecls_.begin();
}

DeclContext::DeclMapConstIter DeclContext::decls_end() const {
  return namedDecls_.end();
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