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

class Sema::StmtChecker : Checker, StmtVisitor<StmtChecker, void>{
  using Inherited = StmtVisitor<StmtChecker, void>;
  friend Inherited;
  public:
    StmtChecker(Sema& sema) : Checker(sema) {}

    void check(Stmt* stmt) {
			visit(stmt);
    }

  private:
    //----------------------------------------------------------------------//
    // Diagnostic methods
    //----------------------------------------------------------------------//
    // The diagnose family of methods are designed to print the most relevant
    // diagnostics for a given situation.
    //
    // Generally speaking, theses methods won't emit diagnostics if 
    // ill formed types are involved, because theses have either:
    //  - been diagnosed already
    //  - will be diagnosed at finalization
    //----------------------------------------------------------------------//
      
    // Diagnoses an expression whose type cannot be used in a condition
    void diagnoseExprCantCond(Expr* expr) {
      getDiags()
        .report(DiagID::sema_cant_use_expr_as_cond, expr->getRange())
        .addArg(expr->getType());
    }

    void diagnoseEmptyReturnStmtInNonVoidFn(ReturnStmt* stmt, Type fnRtrTy) {
      getDiags()
        .report(DiagID::sema_return_with_no_expr, stmt->getRange())
        .addArg(fnRtrTy);
    }

    void diagnoseReturnTypeMistmatch(ReturnStmt* stmt, Expr* expr, 
      Type fnRetTy) {
      Type exprTy = expr->getType();

      if(!Sema::isWellFormed({exprTy, fnRetTy})) return;

      getDiags()
        .report(DiagID::sema_cannot_convert_return_expr, expr->getRange())
        .addArg(exprTy)
        .addArg(fnRetTy)
        .setExtraRange(stmt->getRange());
    }

    void diagnoseUnexpectedRtrExprForNonVoidFn(ReturnStmt* stmt, Expr* expr) {
      getDiags()
        .report(DiagID::sema_unexpected_non_void_rtr_expr, expr->getRange())
        .setExtraRange(stmt->getRange());
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
      
    void visitNullStmt(NullStmt*) {
      // Nothing to check on a NullStmt
    }

    void visitReturnStmt(ReturnStmt* stmt) {
      // Fetch the current FuncDecl
      LocalScope* scope = getSema().getLocalScope();
      assert(scope && "scope shouldn't be nullptr!");
      FuncDecl* fn = scope->getFuncDecl();
      assert(fn && "should have root FuncDecl!");
      // Fetch it's return type
      Type rtrTy = fn->getReturnType();
      bool isVoid = rtrTy->isVoidType();

      // We'll check the stmt depending on whether it has an expression or not.
      if(Expr* expr = stmt->getExpr()) {
        // There is an expression, check it, disallowing downcasts.
        bool succ = getSema().typecheckExprOfType(expr, rtrTy, 
                    /*allowDowncast*/ false);
        if(!succ) {
          // If this function returns void, and has an Expr of a non-void type
          if(isVoid)
            diagnoseUnexpectedRtrExprForNonVoidFn(stmt, expr);
          // non-void function and expr doesn't unify with it's return type
          else
              diagnoseReturnTypeMistmatch(stmt, expr, rtrTy);
        }
        // in all cases, replace the expr after checking it
        stmt->setExpr(expr);
      } 
      else {
        // No expression. If the function's return type isn't void, 
        // this is an error.
        if(!isVoid)
          diagnoseEmptyReturnStmtInNonVoidFn(stmt, rtrTy);
      }
    }

    void visitCompoundStmt(CompoundStmt* stmt) {
			// Just visit the children
      // Note that we don't open a scope for CompoundStmts, because
      // they don't exist in the wild in Fox. They're always attached
      // to Conditions, While, etc.
      for (ASTNode& s : stmt->getNodes()) {
        s = getSema().checkNode(s);
      }
    }

    void visitWhileStmt(WhileStmt* stmt) {
			// Fetch the cond, typecheck it and replace it.
      stmt->setCond(checkCond(stmt->getCond()));
      {
        // Open scope
        auto scope = getSema().openNewScopeRAII();
        // Check the body and replace it
        auto body = getSema().checkNode(stmt->getBody());
        stmt->setBody(body);
      }
    }

		void visitConditionStmt(ConditionStmt* stmt) {
			// Fetch the cond, typecheck it and replace it.
      stmt->setCond(checkCond(stmt->getCond()));
      {
        // Open scope
        auto scope = getSema().openNewScopeRAII();
        // Check the if's body and replace it 
        auto cond_then = getSema().checkNode(stmt->getThen());
        stmt->setThen(cond_then);
      }
			// Check the else's body if there is one and replace it
			if(auto cond_else = stmt->getElse()) {
        // Open scope
        auto scope = getSema().openNewScopeRAII();
        // Check the body and replace it
				cond_else = getSema().checkNode(cond_else);
				stmt->setElse(cond_else);
			}
		}
			
		//----------------------------------------------------------------------//
    // Helper checking methods
    //----------------------------------------------------------------------//
    // Various semantics-related helper methods 
    //----------------------------------------------------------------------//

		// Does the necessary steps to check an expression which
		// is used as a condition. Returns the Expr* that should replace
		// the condition.
		Expr* checkCond(Expr* cond) {
			if(!getSema().typecheckCondition(cond)) {
        if(!(cond->getType()->is<ErrorType>()))
          diagnoseExprCantCond(cond);
      }
			return cond;
		}
};

void Sema::checkStmt(Stmt* stmt) {
	StmtChecker(*this).check(stmt);
}