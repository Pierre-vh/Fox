//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : ASTWalker.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the implementation of the ASTWalker and TypeWalker.
// It is split in 2 parts : Traverse (for ASTWalker) and TypeTraverse 
// (for TypeWalker)
//----------------------------------------------------------------------------//

#include "Fox/AST/ASTWalker.hpp"
#include "Fox/AST/ASTVisitor.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"

using namespace fox;

namespace {
  // The traverse class for Expr, Decl and Stmts
  class Traverse:
  public ASTVisitor<Traverse, Decl*, Expr*, Stmt*> {    
    ASTWalker& walker_;
    public:
      Traverse(ASTWalker& walker) : walker_(walker) {}
       
      // Exprs
      Expr* visitBinaryExpr(BinaryExpr* expr) {
        if (Expr* lhs = expr->getLHS()) {
          if(lhs = doIt(lhs))
            expr->setLHS(lhs);
          else return nullptr;
        }

        if (Expr* rhs = expr->getRHS()) {
          if (rhs = doIt(rhs))
            expr->setRHS(rhs);
          else return nullptr;
        }

        return expr;
      }

      Expr* visitUnaryExpr(UnaryExpr* expr) {
        if (Expr* child = expr->getExpr()) {
          if (child = doIt(child))
            expr->setExpr(child);
          else return nullptr;
        }

        return expr;
      }

      Expr* visitCastExpr(CastExpr* expr) {
        if (Expr* child = expr->getExpr()) {
          if (child = doIt(child))
            expr->setExpr(child);
          else return nullptr;
        }

        return expr;
      }

      Expr* visitArraySubscriptExpr(ArraySubscriptExpr* expr) {
        if (Expr* base = expr->getBase()) {
          if (base = doIt(base))
            expr->setBase(base);
          else return nullptr;
        }

        if (Expr* idx = expr->getIndex()) {
          if (idx = doIt(idx))
            expr->setIndex(idx);
          else return nullptr;
        }

        return expr;
      }

      #define TRIVIAL_EXPR_VISIT(NODE)\
        Expr* visit##NODE(NODE* expr) { return expr; }
      TRIVIAL_EXPR_VISIT(CharLiteralExpr)
      TRIVIAL_EXPR_VISIT(BoolLiteralExpr)
      TRIVIAL_EXPR_VISIT(IntegerLiteralExpr)
      TRIVIAL_EXPR_VISIT(StringLiteralExpr)
      TRIVIAL_EXPR_VISIT(FloatLiteralExpr)
      TRIVIAL_EXPR_VISIT(DeclRefExpr)
      TRIVIAL_EXPR_VISIT(UnresolvedDeclRefExpr)
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

      Expr* visitMemberOfExpr(MemberOfExpr* expr) {
        if (Expr* child = expr->getExpr()) {
          if (child = doIt(child))
            expr->setExpr(child);
          else return nullptr;
        }
        return expr;
      }

      Expr* visitFunctionCallExpr(FunctionCallExpr* expr) {
        if (Expr* callee = expr->getCallee()) {
          if (callee = doIt(callee))
            expr->setCallee(callee);
          else return nullptr;
        }

        for (auto& elem : expr->getArgs()) {
          if (elem) {
            if (Expr* node = doIt(elem))
              elem = node;
            else return nullptr;
          }
        }

        return expr;
      }

      // Decls
      Decl* visitParamDecl(ParamDecl* decl) {
        return decl;
      }

      Decl* visitVarDecl(VarDecl* decl) {
        if (Expr* init = decl->getInitExpr()) {
          if (init = doIt(init))
            decl->setInitExpr(init);
          return nullptr;
        }

        return decl;
      }

      Decl* visitFuncDecl(FuncDecl* decl) {
        for (auto& param : decl->getParams()) {
          if (param) {
            if (Decl* node = doIt(param))
              param = cast<ParamDecl>(node);
            return nullptr;
          }
        }

        if (Stmt* body = decl->getBody()) {
          if (body = doIt(body))
            decl->setBody(cast<CompoundStmt>(body));
          else return nullptr;
        }

        return decl;
      }

      Decl* visitUnitDecl(UnitDecl* decl) {
        for (auto& elem : decl->getDecls()) {
          if (elem) {
            if (Decl* node = doIt(elem))
              elem = node;
            else return nullptr;
          }
        }

        return decl;
      }

      // Stmt
      Stmt* visitNullStmt(NullStmt* stmt) {
        return stmt;
      }

      Stmt* visitReturnStmt(ReturnStmt* stmt) {
        if (Expr* expr = stmt->getExpr()) {
          if (expr = doIt(expr))
            stmt->setExpr(expr);
          else return nullptr;
        }

        return stmt;
      }

      Stmt* visitConditionStmt(ConditionStmt* stmt) {
        if (Expr* cond = stmt->getCond()) {
          if (cond = doIt(cond))
            stmt->setCond(cond);
          else return nullptr;
        }

        if (ASTNode then = stmt->getThen()) {
          if (then = doIt(then))
            stmt->setThen(then);
          else return nullptr;
        }

        if (ASTNode elsestmt = stmt->getElse()) {
          if (elsestmt = doIt(elsestmt))
            stmt->setElse(elsestmt);
          else return nullptr;
        }

        return stmt;
      }

      Stmt* visitCompoundStmt(CompoundStmt* stmt) {
        for (auto& elem: stmt->getNodes()) {
          if (elem) {
            if (ASTNode node = doIt(elem))
              elem = node;
            else return nullptr;
          }
        }

        return stmt;
      }

      Stmt* visitWhileStmt(WhileStmt* stmt) {
        if (Expr* cond = stmt->getCond()) {
          if (cond = doIt(cond))
            stmt->setCond(cond);
          else return nullptr;
        }

        if (ASTNode node = stmt->getBody()) {
          if (node = doIt(node))
            stmt->setBody(node);
          else return nullptr;
        }

        return stmt;
      }

      // doIt method for expression: handles call to the walker &
      // requests visitation of the children of a given node.
      Expr* doIt(Expr* expr) {
        // Let the walker handle the pre visitation stuff.
        auto rtr = walker_.handleExprPre(expr);

        // Return if we have a nullptr or if we're instructed
        // to not visit the children.
        if (!rtr.first || !rtr.second)
          return rtr.first;

        // visit the node's childre, and if the traversal wasn't aborted,
        // let the walker handle post visitation stuff.
        if (expr = visit(rtr.first))
          expr = walker_.handleExprPost(expr);

        return expr;
      }

      // doIt method for declarations: handles call to the walker &
      // requests visitation of the children of a given node.
      Decl* doIt(Decl* expr) {
        // Let the walker handle the pre visitation stuff.
        auto rtr = walker_.handleDeclPre(expr);

        // Return if we have a nullptr or if we're instructed
        // to not visit the children.
        if (!rtr.first || !rtr.second)
          return rtr.first;

        // visit the node's childre, and if the traversal wasn't aborted,
        // let the walker handle post visitation stuff.
        if (expr = visit(rtr.first))
          expr = walker_.handleDeclPost(expr);

        return expr;
      }

      // doIt method for statements: handles call to the walker &
      // requests visitation of the children of a given node.
      Stmt* doIt(Stmt* expr) {
        // Let the walker handle the pre visitation stuff.
        auto rtr = walker_.handleStmtPre(expr);

        // Return if we have a nullptr or if we're instructed
        // to not visit the children.
        if (!rtr.first || !rtr.second)
          return rtr.first;

        // visit the node's children, and if the traversal wasn't aborted,
        // let the walker handle post visitation stuff.
        if (expr = visit(rtr.first))
          expr = walker_.handleStmtPost(expr);

        return expr;
      }

      ASTNode doIt(ASTNode node) {
        if (Decl* decl = node.dyn_cast<Decl*>())
          return doIt(decl);
        if (Stmt* stmt = node.dyn_cast<Stmt*>())
          return doIt(stmt);
        if (Expr* expr = node.dyn_cast<Expr*>())
          return doIt(expr);

        fox_unreachable("Unknown node contained in ASTNode");
      }
  };

  // TypeTraverse, the traverse class for Typess
  class TypeTraverse : public TypeVisitor<TypeTraverse, bool> {
    TypeWalker &walker_;
    public:
      TypeTraverse(TypeWalker& walker) : walker_(walker) {}

      // doIt method for types
      bool doIt(Type type) {
        // Call the walker, abort if failed.
        if (!walker_.handleTypePre(type))
          return false;

        // Visit the children
        if (visit(type))
          // Call the walker (post)
          return walker_.handleTypePost(type);
        return false;
      }

      bool visitPrimitiveType(PrimitiveType*) {
        return true;
      }

      bool visitArrayType(ArrayType* type) {
        if (Type elem = type->getElementType())
          return doIt(elem);
        return true;
      }

      bool visitLValueType(LValueType* type) {
        if (Type ty = type->getType())
          return doIt(ty);
        return true;
      }

      bool visitErrorType(ErrorType*) {
        return true;
      }

      bool visitCellType(CellType* type) {
        if (Type sub = type->getSubst())
          return doIt(sub);
        return true;
      }
  };

} // End anonymous namespace

// ASTWalker

ASTNode ASTWalker::walk(ASTNode node) {
  if (Decl* decl = node.dyn_cast<Decl*>())
    return walk(decl);
  if (Stmt* stmt = node.dyn_cast<Stmt*>())
    return walk(stmt);
  if (Expr* expr = node.dyn_cast<Expr*>())
    return walk(expr);

  fox_unreachable("Unknown node contained in ASTNode");
}

Expr* ASTWalker::walk(Expr* expr) {
  return Traverse(*this).doIt(expr);
}

Decl* ASTWalker::walk(Decl* decl) {
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

std::pair<Decl*, bool> ASTWalker::handleDeclPre(Decl* decl) {
  return { decl, true };
}

Decl* ASTWalker::handleDeclPost(Decl* decl) {
  return decl;
}

// TypeWalker

bool TypeWalker::walk(Type type) {
  return TypeTraverse(*this).doIt(type);
}

bool TypeWalker::handleTypePre(Type) {
  return true;
}

bool TypeWalker::handleTypePost(Type) {
  return true;
}
