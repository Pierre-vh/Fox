//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : SemaStmt.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file implements semantic analysis for statements.
//----------------------------------------------------------------------------//

#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/Type.hpp"
#include "Fox/AST/Types.hpp"
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
    // "diagnose" methods
    //----------------------------------------------------------------------//
    // Methods designed to diagnose specific situations.
    //----------------------------------------------------------------------//
      
    /// Diagnoses an invalid condition expression
    void diagnoseExprCantCond(Expr* expr) {
      if(Sema::isWellFormed(expr->getType()))
        diagEngine
          .report(DiagID::cant_use_expr_as_cond, expr->getSourceRange())
          .addArg(expr->getType());
    }

    /// Diagnoses an empty return statement in a non-void function
    void diagnoseEmptyReturnStmtInNonVoidFn(ReturnStmt* stmt, Type fnRtrTy) {
      diagEngine
        .report(DiagID::return_with_no_expr, stmt->getSourceRange())
        .addArg(fnRtrTy);
    }

    /// Diagnoses a return type mismatch 
    /// (type of the return expr != return type of func)
    void diagnoseReturnTypeMistmatch(ReturnStmt* stmt, Expr* expr, 
                                     Type fnRetTy) {
      // Don't diagnose if the return expression's type isn't well formed
      if(!Sema::isWellFormed(expr->getType())) return;

      Type exprTy = expr->getType();

      if(!Sema::isWellFormed({exprTy, fnRetTy})) return;

      diagEngine
        .report(DiagID::cannot_convert_return_expr, expr->getSourceRange())
        .addArg(exprTy)
        .addArg(fnRetTy)
        .setExtraRange(stmt->getReturnSourceRange());
    }

    /// Diagnoses a return type mismatch for void functions
    /// (type of the return expr != void)
    void diagnoseUnexpectedRtrExprForNonVoidFn(ReturnStmt* stmt, Expr* expr) {
      // Don't diagnose if the return expression's type isn't well formed
      if(!Sema::isWellFormed(expr->getType())) return;
      diagEngine
        .report(DiagID::unexpected_non_void_rtr_expr, expr->getSourceRange())
        .setExtraRange(stmt->getReturnSourceRange());
    }

    //----------------------------------------------------------------------//
    // "visit" methods
    //----------------------------------------------------------------------//
    // Does semantic analysis on a single Stmt
    //----------------------------------------------------------------------//

    void visitReturnStmt(ReturnStmt* stmt) {
      // Fetch the current FuncDecl
      DeclContext* dc = sema.getDeclCtxt();
      assert(dc && "no active decl context!");
      FuncDecl* fn = dyn_cast<FuncDecl>(dc);
      assert(fn && "ReturnStmt outside a FuncDecl?");
      // Fetch it's return type
      Type rtrTy = fn->getReturnTypeLoc().getType();
      bool isVoid = rtrTy->isVoidType();

      if(Expr* expr = stmt->getExpr()) {
        // Check the return expression
        if(!sema.typecheckExprOfType(expr, rtrTy)) {
          // Type mismatch: diagnose
          if(isVoid)
            diagnoseUnexpectedRtrExprForNonVoidFn(stmt, expr);
          else
            diagnoseReturnTypeMistmatch(stmt, expr, rtrTy);
        }
        // Replace the return expression
        stmt->setExpr(expr);
      } 
      // No return expression & expression doesn't return void
      else if(!isVoid)
        diagnoseEmptyReturnStmtInNonVoidFn(stmt, rtrTy);
    }

    void visitCompoundStmt(CompoundStmt* stmt) {
      for (ASTNode& s : stmt->getNodes()) {
        s = checkNode(s);
      }
    }

    void visitWhileStmt(WhileStmt* stmt) {
      // Check & replace the cond
      stmt->setCond(checkCond(stmt->getCond()));
      // Check the body
      visitCompoundStmt(stmt->getBody());
    }

    void visitConditionStmt(ConditionStmt* stmt) {
      // Check & replace the cond
      stmt->setCond(checkCond(stmt->getCond()));
      // Check the if's body
      visitCompoundStmt(stmt->getThen());
	    // Check the else's body if present
	    if(Stmt* elseBody = stmt->getElse())
		    visit(elseBody);
    }
			
    //----------------------------------------------------------------------//
    // Helper checking methods
    //----------------------------------------------------------------------//
    // Various semantics-related helper methods 
    //----------------------------------------------------------------------//

    ASTNode checkNode(ASTNode node) {
      assert(node && "node is null");
      if(node.is<Decl*>())
        sema.checkDecl(node.get<Decl*>());
      else if(node.is<Expr*>())
        node = sema.typecheckExpr(node.get<Expr*>());
      else {
        assert(node.is<Stmt*>() && "unknown ASTNode kind");
        Stmt* stmt = node.get<Stmt*>();
        visit(stmt);
      }
      assert(node && "node became null");
      return node;
    }

		/// Does the necessary steps to check an expression which
		/// is used as a condition. 
    /// \returns the expr that should replace \p cond
		Expr* checkCond(Expr* cond) {
			if(!sema.typecheckCondition(cond)) {
        if(!(cond->getType()->is<ErrorType>()))
          diagnoseExprCantCond(cond);
      }
			return cond;
		}
};

void Sema::checkStmt(Stmt* stmt) {
	StmtChecker(*this).check(stmt);
}