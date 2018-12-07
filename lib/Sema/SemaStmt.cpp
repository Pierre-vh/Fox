//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : SemaStmt.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file implements semantic analysis for statements.
//----------------------------------------------------------------------------//

#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/AST/ASTWalker.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"

using namespace fox;

namespace {
  class StmtChecker : StmtVisitor<StmtChecker, bool>{
    using Inherited = StmtVisitor<StmtChecker, bool>;
    friend class Inherited;
    Sema& sema_;
    DiagnosticEngine& diags_;
    ASTContext& ctxt_;
    public:
      StmtChecker(Sema& sema) : sema_(sema),
        diags_(sema.getDiagnosticEngine()), ctxt_(sema.getASTContext()) {}

      ASTContext& getCtxt() {
        return ctxt_;
      }

      DiagnosticEngine& getDiags() {
        return diags_;
      }

      Sema& getSema() {
        return sema_;
      }

      bool check(Stmt* stmt) {
        return visit(stmt);
      }

    private:
      //----------------------------------------------------------------------//
      // Diagnostic methods
      //----------------------------------------------------------------------//
      // The diagnose family of methods are designed to print the most relevant
      // diagnostics for a given situation.
      //----------------------------------------------------------------------//
      
      // Diagnoses an expression whose type cannot be used in a condition
      void diagnoseExprCantCond(Expr* expr) {
        getDiags()
          .report(DiagID::sema_cant_use_expr_as_cond, expr->getRange())
          .addArg(expr->getType());
      }

      //----------------------------------------------------------------------//
      // "visit" methods
      //----------------------------------------------------------------------//
      // Theses visit() methods will perform the necessary tasks to check a
      // single statement.
      //
      // Theses methods may call visit on the children of the statement. 
      // (Visitation is handled by the visit methods, and not through a 
      // ASTWalker like in the ExprChecker (SemaExpr.cpp))
      //----------------------------------------------------------------------//
      
      bool visitNullStmt(NullStmt*) {
        // Nothing to check on a NullStmt = Always successful.
        return true;
      }

      bool visitReturnStmt(ReturnStmt*) {
        // We need to know the current function's signature
        // to typecheck this.
        fox_unimplemented_feature("Return statements checking");
      }

      bool visitCompoundStmt(CompoundStmt* stmt) {
        // just visit the children and eturn true if the visit method returned
        // true for all of them
        bool succ = true;
        for (ASTNode& s : stmt->getNodes()) {
          // Replace if needed
          auto pair = getSema().checkNode(s);
          s = pair.second;
          succ &= pair.first;
        }
        return succ;
      }

      bool visitWhileStmt(WhileStmt* stmt) {
        {
          Type boolTy = PrimitiveType::getBool(getCtxt());
          Expr* e = stmt->getCond();
          auto pair = getSema().typecheckExprOfType(e, boolTy);
          stmt->setCond(pair.second);
          // TODO: Switch on pair.first
        }
        return true;
      }
  };
} // anonymous namespace

bool Sema::checkStmt(Stmt* stmt) {
  return StmtChecker(*this).check(stmt);
}