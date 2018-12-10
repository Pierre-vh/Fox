//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Sema.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file implements Sema methods that aren't tied to Expression,
//  Statements, Declarations or Types.
//----------------------------------------------------------------------------//

#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/ASTNode.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

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
  if (Decl* d = node.dyn_cast<Decl*>())
    fox_unimplemented_feature("Decl checking");
  fox_unreachable("unknown ASTNode kind");
}

Sema::RAIIDeclCtxt Sema::setDeclCtxtRAII(DeclContext* dc) {
  return RAIIDeclCtxt(*this, dc);
}

DeclContext* Sema::getDeclCtxt() const {
  return currentDC_;
}

bool Sema::hasDeclCtxt() const {
  return (currentDC_ != nullptr);
}

Sema::RAIILocalScope Sema::enterNewLocalScopeRAII() {
  return RAIILocalScope(*this);
}

LocalScope* Sema::getLocalScope() const {
  return localScope_;
}

bool Sema::hasLocalScope() const {
  return (bool)localScope_;
}