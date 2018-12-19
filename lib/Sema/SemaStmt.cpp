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
  friend class Inherited;
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
      
    void visitNullStmt(NullStmt*) {
      // Nothing to check on a NullStmt
    }

    void visitReturnStmt(ReturnStmt*) {
      // We need to know the current function's signature
      // to check this -> Need Decl checking to be done.
      fox_unimplemented_feature("Return statements checking");
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
        auto scope = getSema().enterLocalScopeRAII();
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
        auto scope = getSema().enterLocalScopeRAII();
        // Check the if's body and replace it 
        auto cond_then = getSema().checkNode(stmt->getThen());
        stmt->setThen(cond_then);
      }
			// Check the else's body if there is one and replace it
			if(auto cond_else = stmt->getElse()) {
        // Open scope
        auto scope = getSema().enterLocalScopeRAII();
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
			using CER = Sema::CheckedExprResult;
			Type boolTy = PrimitiveType::getBool(getCtxt());
			auto condRes = getSema().typecheckExprOfType(cond, boolTy);
			// Only emit a diagnostic if it's not an ErrorType
			if(condRes.first == CER::NOk)
					diagnoseExprCantCond(cond);
			return cond;
		}
};

void Sema::checkStmt(Stmt* stmt) {
	StmtChecker(*this).check(stmt);
}