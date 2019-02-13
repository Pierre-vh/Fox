//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Sema.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file implements Sema methods that don't contain any AST-checking logic
//  as well as  the implementation of most subobjects of Sema.
//----------------------------------------------------------------------------//

#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/ASTNode.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/ASTContext.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// Sema Methods
//----------------------------------------------------------------------------//

Sema::Sema(ASTContext& ctxt) : ctxt_(ctxt) {}

DiagnosticEngine& Sema::getDiagnosticEngine() {
  return ctxt_.diagEngine;
}

ASTContext& Sema::getASTContext() {
  return ctxt_;
}

//----------------------------------------------------------------------------//
// RAIIDeclCtxt
//----------------------------------------------------------------------------//

Sema::RAIIDeclCtxt Sema::enterDeclCtxtRAII(DeclContext* dc) {
  return RAIIDeclCtxt(*this, dc);
}

DeclContext* Sema::getDeclCtxt() const {
  return currentDC_;
}

bool Sema::hasDeclCtxt() const {
  return (currentDC_ != nullptr);
}

//----------------------------------------------------------------------------//
// RAIILocalScope
//----------------------------------------------------------------------------//

Sema::RAIILocalScope Sema::openNewScopeRAII() {
  assert(localScope_ 
    && "LocalScope cannot be nullptr. Use enterFuncScopeRAII!");
  return RAIILocalScope(*this);
}

Sema::RAIILocalScope Sema::enterFuncScopeRAII(FuncDecl* fn) {
  assert(fn 
    && "null fn");
  assert(!localScope_ 
    && "LocalScope must be nullptr. Use openNewScopeRAII!");
  return RAIILocalScope(*this, fn);
}

LocalScope* Sema::getLocalScope() const {
  return localScope_;
}

bool Sema::hasLocalScope() const {
  return (bool)localScope_;
}