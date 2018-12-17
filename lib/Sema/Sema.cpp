//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
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

using namespace fox;

//----------------------------------------------------------------------------//
// Sema Methods
//----------------------------------------------------------------------------//

Sema::Sema(ASTContext& ctxt, DiagnosticEngine& diags) :
  ctxt_(ctxt), diags_(diags) {

}

DiagnosticEngine& Sema::getDiagnosticEngine() {
  return diags_;
}

ASTContext& Sema::getASTContext() {
  return ctxt_;
}

ASTNode Sema::checkNode(ASTNode node) {
	assert(!node.isNull() && 
		"node cannot be null!");
  if (Expr* e = node.dyn_cast<Expr*>())
    return typecheckExpr(e);
  if (Stmt* s = node.dyn_cast<Stmt*>()) {
		checkStmt(s);
		return node;
	}
  if (Decl* d = node.dyn_cast<Decl*>()) {
    checkDecl(d);
    return node;
  }
  fox_unreachable("unknown ASTNode kind");
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

Sema::RAIILocalScope Sema::enterLocalScopeRAII() {
  return RAIILocalScope(*this);
}

LocalScope* Sema::getLocalScope() const {
  return localScope_;
}

bool Sema::hasLocalScope() const {
  return (bool)localScope_;
}