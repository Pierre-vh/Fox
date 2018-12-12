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

void DeclContext::addDecl(NamedDecl* decl) {
  assert(decl  && "Declaration cannot be null!");
  Identifier name = decl->getIdentifier();
  assert(name && "Declaration must have a non-null Identifier to be "
    "inserted in the DeclContext");
  assert(decl->getRange() && "Declaration must have valid source location"
    "information to be inserted in the DeclContext");
  assert(!decl->isLocal() && "Can't add local declarations in a DeclContext!");
  decls_.insert({name, decl});
}

const DeclContext::MapTy& DeclContext::getDeclsMap() const {
  return decls_;
}

bool DeclContext::hasParentDeclCtxt() const {
  return parentAndKind_.getPointer() != nullptr;
}

DeclContext* DeclContext::getParentDeclCtxt() const {
  return parentAndKind_.getPointer();
}

DeclContext::LexicalDeclsTy DeclContext::getLexicalDecls(FileID forFile) {
  // Create the vector with enough space to hold every value.
  LexicalDeclsTy rtr;
  // Populate it
  for(auto& elem : decls_) {
    if(elem.second->getFile() == forFile)
      rtr.push_back(elem.second);
  }
  // Predicate for comparing the decls using their begin locs.
  struct Predicate {
    bool operator()(Decl* a, Decl* b){
      // Since we know both files are equal, we can simply iterate
      // like this.
      auto aIdx = a->getRange().getBegin().getIndex();
      auto bIdx = b->getRange().getBegin().getIndex();
      return aIdx < bIdx;
    }
  };
  // Sort it
  std::sort(rtr.begin(), rtr.end(), Predicate());
  return rtr;
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