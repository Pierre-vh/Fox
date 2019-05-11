//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : ASTVisitor.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This declares and implements the ASTVisitor class.
//----------------------------------------------------------------------------//

#pragma once

#include "Decl.hpp"
#include "Stmt.hpp"
#include "Expr.hpp"
#include "ASTNode.hpp"
#include "Fox/Common/Errors.hpp"
#include <utility>

namespace fox {
  /// A basic visitor class for Expr, Stmt and Decls.
  template<
    typename Derived, 
    typename DeclRtrTy,
    typename ExprRtrTy,
    typename StmtRtrTy,
    typename ... Args
  >
  class ASTVisitor {
    public:
      /// Visits an ASTNode
      void visit(ASTNode node, Args... args) {
        assert(!node.isNull() && "Cannot be used on a null pointer");
        if (node.is<Decl*>())
          visit(node.get<Decl*>(), ::std::forward<Args>(args)...);
        else if (node.is<Expr*>())
          visit(node.get<Expr*>(), ::std::forward<Args>(args)...);
        else if (node.is<Stmt*>())
          visit(node.get<Stmt*>(), ::std::forward<Args>(args)...);
        else
          fox_unreachable("Unsupported ASTNode variant");
      }

      /// Visits a declaration
      DeclRtrTy visit(Decl* decl, Args... args) {
        assert(decl && "Cannot be used on a null pointer");
        switch (decl->getKind()) {
          #define DECL(ID,PARENT)                   \
            case DeclKind::ID:                      \
              return static_cast<Derived*>(this)->  \
                visit##ID(static_cast<ID*>(decl),   \
                  ::std::forward<Args>(args)...);
          #include "DeclNodes.def"
          default:
            fox_unreachable("Unknown node");
        }
      }

      /// Visits a statement
      StmtRtrTy visit(Stmt* stmt, Args... args) {
        assert(stmt && "Cannot be used on a null pointer");
        switch (stmt->getKind()) {
          #define STMT(ID,PARENT)                   \
            case StmtKind::ID:                      \
              return static_cast<Derived*>(this)->  \
                visit##ID(static_cast<ID*>(stmt),   \
                  ::std::forward<Args>(args)...);
          #include "StmtNodes.def"
          default:
            fox_unreachable("Unknown node");
        }
      }

      /// Visits an expression
      ExprRtrTy visit(Expr* expr, Args... args) {
        assert(expr && "Cannot be used on a null pointer");
        switch (expr->getKind()) {
          #define EXPR(ID,PARENT)                    \
            case ExprKind::ID:                       \
              return static_cast<Derived*>(this)->   \
                visit##ID(static_cast<ID*>(expr),    \
                  ::std::forward<Args>(args)...);
          #include "ExprNodes.def"
          default:
            fox_unreachable("Unknown node");
        }
      }

      // Sometimes, visit methods might not return (e.g. call fox_unreachable)
      // and MSVC is sort of pedantic about this and will complain about 
      // unreachable code in the .def file. 
      // The error it emits is unhelpful and not useful, so we're going to
      // disable it for visit methods.
      #pragma warning(push)
      #pragma warning(disable:4702)

      // VisitXXX Methods
      // The base implementations just chain back to the parent class, 
      // so visitors can just implement the parent class or an abstract class
      // and still handle every derived class!
      // 
      // Note that we don't provide a default implementation of 
      // "visitExpr/visitStmt/visitDecl", this is because we require full
      // coverage of the AST by the visitors.
      #define VISIT_METHOD(RTRTYPE, NODE, PARENT)             \
      RTRTYPE visit##NODE(NODE* node, Args... args){          \
        return static_cast<Derived*>(this)->                  \
          visit##PARENT(node, ::std::forward<Args>(args)...); \
      }

      // Decls
      #define DECL(ID,PARENT) VISIT_METHOD(DeclRtrTy, ID, PARENT)
      #define ABSTRACT_DECL(ID,PARENT) VISIT_METHOD(DeclRtrTy, ID, PARENT)
      #include "DeclNodes.def"

      // Stmts
      #define STMT(ID,PARENT) VISIT_METHOD(StmtRtrTy, ID, PARENT)
      #define ABSTRACT_STMT(ID,PARENT) VISIT_METHOD(StmtRtrTy, ID, PARENT)
      #include "StmtNodes.def"

      // Exprs
      #define EXPR(ID,PARENT) VISIT_METHOD(ExprRtrTy, ID, PARENT)
      #define ABSTRACT_EXPR(ID,PARENT) VISIT_METHOD(ExprRtrTy, ID, PARENT)
      #include "ExprNodes.def"

      #undef VISIT_METHOD
      #pragma warning(pop)
  };

  /// A Simple AST Visitor that uses the same return type
  /// for each node kind
  template<typename Derived, typename RtrTy, typename ... Args>
  using SimpleASTVisitor = ASTVisitor<Derived, RtrTy, RtrTy, RtrTy, Args...>;

  /// An AST Visitor that only visits declarations
  template<typename Derived, typename RtrTy, typename ... Args>
  using DeclVisitor = ASTVisitor<Derived, RtrTy, void, void, Args...>;

  /// An AST Visitor that only visits Expressions
  template<typename Derived, typename RtrTy, typename ... Args>
  using ExprVisitor = ASTVisitor<Derived, void, RtrTy, void, Args...>;

  /// An AST Visitor that only visits Statements
  template<typename Derived, typename RtrTy, typename ... Args>
  using StmtVisitor = ASTVisitor<Derived, void, void, RtrTy, Args...>;
}
