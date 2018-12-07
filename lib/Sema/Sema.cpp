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

std::pair<bool, ASTNode> Sema::checkNode(ASTNode node) {
  if (Expr* e = node.getIf<Expr>())
    return typecheckExpr(e);
  if (Stmt* s = node.getIf<Stmt>())
    return { checkStmt(s), s };
  if (Decl* d = node.getIf<Decl>())
    fox_unimplemented_feature("Decl checking");
  fox_unreachable("unknown ASTNode kind");
}