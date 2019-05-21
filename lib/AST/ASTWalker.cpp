//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : ASTWalker.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//


#include "Fox/AST/ASTWalker.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"

using namespace fox;

namespace {
  /// The traverse class for Expr, Decl and Stmts
  /// This is an implementation class for the \ref ASTWalker.
  /// Visit methods return non-null (or true) on success, nullptr (or false)
  /// when the walk should be aborted.
  class Traverse:
  public ASTVisitor<Traverse, bool, Expr*, Stmt*> {    
    public:
      Traverse(ASTWalker& walker) : walker(walker) {}

      ASTWalker& walker;

      //----------------------------------------------------------------------//
      // Expressions
      //----------------------------------------------------------------------//

      Expr* visitBinaryExpr(BinaryExpr* expr) {
        if (Expr* lhs = expr->getLHS()) {
          if((lhs = doIt(lhs)))
            expr->setLHS(lhs);
          else 
            return nullptr;
        }

        if (Expr* rhs = expr->getRHS()) {
          if ((rhs = doIt(rhs)))
            expr->setRHS(rhs);
          else 
            return nullptr;
        }

        return expr;
      }

      Expr* visitUnaryExpr(UnaryExpr* expr) {
        if (Expr* child = expr->getChild()) {
          if ((child = doIt(child)))
            expr->setChild(child);
          else 
            return nullptr;
        }

        return expr;
      }

      Expr* visitCastExpr(CastExpr* expr) {
        if (Expr* child = expr->getChild()) {
          if ((child = doIt(child)))
            expr->setChild(child);
          else 
            return nullptr;
        }

        return expr;
      }

      Expr* visitSubscriptExpr(SubscriptExpr* expr) {
        if (Expr* base = expr->getBase()) {
          if ((base = doIt(base)))
            expr->setBase(base);
          else 
            return nullptr;
        }

        if (Expr* idx = expr->getIndex()) {
          if ((idx = doIt(idx)))
            expr->setIndex(idx);
          else 
            return nullptr;
        }

        return expr;
      }

      #define TRIVIAL_EXPR_VISIT(NODE)\
        Expr* visit##NODE(NODE* expr) { return expr; }
      TRIVIAL_EXPR_VISIT(CharLiteralExpr)
      TRIVIAL_EXPR_VISIT(BoolLiteralExpr)
      TRIVIAL_EXPR_VISIT(IntegerLiteralExpr)
      TRIVIAL_EXPR_VISIT(StringLiteralExpr)
      TRIVIAL_EXPR_VISIT(DoubleLiteralExpr)
      TRIVIAL_EXPR_VISIT(DeclRefExpr)
      TRIVIAL_EXPR_VISIT(UnresolvedDeclRefExpr)
      TRIVIAL_EXPR_VISIT(ErrorExpr)
      #undef TRIVIAL_EXPR_VISIT
      
      Expr* visitArrayLiteralExpr(ArrayLiteralExpr* expr) {
        for (auto& elem : expr->getExprs()) {
          if (elem) {
            if (Expr* node = doIt(elem))
              elem = node;
            else return nullptr;
          }
        }

        return expr;
      }

      Expr* visitUnresolvedDotExpr(UnresolvedDotExpr* expr) {
        if (Expr* child = expr->getExpr()) {
          if ((child = doIt(child)))
            expr->setExpr(child);
          else 
            return nullptr;
        }
        return expr;
      }

      Expr* visitCallExpr(CallExpr* expr) {
        if (Expr* callee = expr->getCallee()) {
          if ((callee = doIt(callee)))
            expr->setCallee(callee);
          else 
            return nullptr;
        }

        for (auto& elem : expr->getArgs()) {
          if (elem) {
            if (Expr* node = doIt(elem))
              elem = node;
            else 
              return nullptr;
          }
        }

        return expr;
      }

      //----------------------------------------------------------------------//
      // Declarations
      //----------------------------------------------------------------------//

      bool visitParamDecl(ParamDecl*) {
        return true;
      }

      bool visitVarDecl(VarDecl* decl) {
        if (Expr* init = decl->getInitExpr())
          return doIt(init);
        return true;
      }

      bool visitFuncDecl(FuncDecl* decl) {
        for (ParamDecl* param : *decl->getParams()) {
          if (param) {
            if (!doIt(param))
              return false;
          }
        }

        if (Stmt* body = decl->getBody()) {
          if ((body = doIt(body)))
            decl->setBody(cast<CompoundStmt>(body));
          else 
            return false;
        }

        return true;
      }

      bool visitBuiltinFuncDecl(BuiltinFuncDecl* decl) {
        return decl;
      }

      bool visitUnitDecl(UnitDecl* decl) {
        for (Decl* elem : decl->getDecls()) {
          if (!doIt(elem))
            return false;
        }
        return true;
      }

      //----------------------------------------------------------------------//
      // Statements
      //----------------------------------------------------------------------//

      Stmt* visitReturnStmt(ReturnStmt* stmt) {
        if (Expr* expr = stmt->getExpr()) {
          if ((expr = doIt(expr)))
            stmt->setExpr(expr);
          else 
            return nullptr;
        }

        return stmt;
      }

      Stmt* visitConditionStmt(ConditionStmt* stmt) {
        if (Expr* cond = stmt->getCond()) {
          if ((cond = doIt(cond)))
            stmt->setCond(cond);
          else 
            return nullptr;
        }

        if (Stmt* then = stmt->getThen()) {
          if ((then = doIt(then)))
            stmt->setThen(cast<CompoundStmt>(then));
          else 
            return nullptr;
        }

        if (Stmt* elseBody = stmt->getElse()) {
          if ((elseBody = doIt(elseBody))) {
            stmt->setElse(elseBody);
          }
          else 
            return nullptr;
        }

        return stmt;
      }

      Stmt* visitCompoundStmt(CompoundStmt* stmt) {
        for (ASTNode& elem: stmt->getNodes()) {
          if (elem) {
            if (ASTNode node = doIt(elem))
              elem = node;
            else 
              return nullptr;
          }
        }

        return stmt;
      }

      Stmt* visitWhileStmt(WhileStmt* stmt) {
        if (Expr* cond = stmt->getCond()) {
          if ((cond = doIt(cond)))
            stmt->setCond(cond);
          else 
            return nullptr;
        }

        if (Stmt* body = stmt->getBody()) {
          if (body = doIt(body))
            stmt->setBody(cast<CompoundStmt>(body));
          else 
            return nullptr;
        }

        return stmt;
      }

      /// doIt method for expressions
      /// \returns nullptr if the walk should be terminated
      Expr* doIt(Expr* expr) {
        auto rtr = walker.handleExprPre(expr);
        // Return if we have a nullptr or if we shouldn't visit the children
        if (!rtr.first || !rtr.second)
          return rtr.first;
        // visit the children
        if ((expr = visit(rtr.first))) 
          // On success, call handleExprPost
          expr = walker.handleExprPost(expr);

        return expr;
      }

      /// doIt method for declarations
      /// \returns false if the walk should be terminated
      bool doIt(Decl* decl) {
        // When handleDeclPre returns false, this means we should skip children,
        // not that the walk failed, so return "true" instead of false.
        if (!walker.handleDeclPre(decl))
          return false;
        if (visit(decl))
          return walker.handleDeclPost(decl);
        return false;
      }

      /// doIt method for statements
      /// \returns nullptr if the walk should be terminated.
      Stmt* doIt(Stmt* stmt) {
        auto rtr = walker.handleStmtPre(stmt);
        // Return if we have a nullptr or if we're instructed
        // to not visit the children.
        if (!rtr.first || !rtr.second)
          return rtr.first;
        // visit the node's children and call handleStmtPost if needed
        if ((stmt = visit(rtr.first)))
          stmt = walker.handleStmtPost(stmt);
        return stmt;
      }

      /// doIt method for ASTNodes : dispatchs to the correct
      /// 'doIt' method.
      /// \returns nullptr if the walk should be terminated.
      ASTNode doIt(ASTNode node) {
        if (Decl* decl = node.dyn_cast<Decl*>())
          // Important: for Decls, never replace them, always return the arg.
          return doIt(decl) ? decl : nullptr;
        if (Stmt* stmt = node.dyn_cast<Stmt*>())
          return doIt(stmt);
        if (Expr* expr = node.dyn_cast<Expr*>())
          return doIt(expr);
        fox_unreachable("Unknown ASTNode kind");
      }
  };
} // end anonymous namespace

void ASTWalker::walk(ASTNode node) {
  Traverse(*this).doIt(node);
}

Expr* ASTWalker::walk(Expr* expr) {
  return Traverse(*this).doIt(expr);
}

bool ASTWalker::walk(Decl* decl) {
  return Traverse(*this).doIt(decl);
}

Stmt* ASTWalker::walk(Stmt* stmt) {
  return Traverse(*this).doIt(stmt);
}

std::pair<Expr*, bool> ASTWalker::handleExprPre(Expr* expr) {
  return { expr, true };
}

Expr* ASTWalker::handleExprPost(Expr* expr) {
  return expr;
}

std::pair<Stmt*, bool> ASTWalker::handleStmtPre(Stmt* stmt) {
  return { stmt, true };
}

Stmt* ASTWalker::handleStmtPost(Stmt* stmt) {
  return stmt;
}

bool ASTWalker::handleDeclPre(Decl*) {
  return true;
}

bool ASTWalker::handleDeclPost(Decl*) {
  return true;
}
