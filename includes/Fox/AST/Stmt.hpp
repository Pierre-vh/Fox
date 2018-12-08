//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Stmt.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file declares the interface Stmt and it's derived classes.              
//----------------------------------------------------------------------------//

#pragma once

#include "ASTAligns.hpp"
#include "Fox/Common/Source.hpp"
#include "ASTNode.hpp"
#include <vector>

namespace fox {
  // Kinds of Statements
  enum class StmtKind : std::uint8_t {
    #define STMT(ID,PARENT) ID,
    #define STMT_RANGE(ID,FIRST,LAST) First_##ID = FIRST, Last_##ID = LAST,
    #include "StmtNodes.def"
  };

  // Forward Declarations
  class Decl;
  class Expr;
  class ASTContext;

  // Stmt
  //    Common base for every statement
  class alignas(align::StmtAlignement) Stmt {
    public:
      StmtKind getKind() const;

      void setRange(SourceRange range);
      SourceRange getRange() const;

      void dump() const;

      // Prohibit the use of builtin placement new & delete
      void *operator new(std::size_t) throw() = delete;
      void operator delete(void *) throw() = delete;
      void* operator new(std::size_t, void*) = delete;

      // Only allow allocation through the ASTContext
      void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(Stmt));

      // Companion operator delete to silence C4291 on MSVC
      void operator delete(void*, ASTContext&, std::uint8_t) {}

    protected:
      Stmt(StmtKind kind, SourceRange range);

    private:
      SourceRange range_;
      const StmtKind kind_;
  };

  // NullStmt
  //    A null statement ';'
  //    Often used as the body of a condition/loop
  class NullStmt : public Stmt {
    public:
      NullStmt();
      NullStmt(SourceLoc semiLoc);

      void setSemiLoc(SourceLoc semiLoc);
      SourceLoc getSemiLoc() const;

      static bool classof(const Stmt* stmt) {
        return (stmt->getKind() == StmtKind::NullStmt);
      }
  };

  // ReturnStmt
  //    A return statement
  class ReturnStmt : public Stmt {
    public:
      ReturnStmt();
      ReturnStmt(Expr* rtr_expr, SourceRange range);

      void setExpr(Expr* e);
      Expr* getExpr();
      const Expr* getExpr() const;
      bool hasExpr() const;

      static bool classof(const Stmt* stmt) {
        return (stmt->getKind() == StmtKind::ReturnStmt);
      }

    private:
      Expr* expr_ = nullptr;
  };

  // ConditionStmt
  //    if-then-else conditional statement
  class ConditionStmt : public Stmt {
    public:
      ConditionStmt();
      ConditionStmt(Expr* cond, ASTNode then, ASTNode elsenode,
        SourceRange range, SourceLoc ifHeaderEndLoc);

      bool isValid() const;

      void setCond(Expr* expr);
      Expr* getCond();
      const Expr* getCond() const;

      void setThen(ASTNode node);
      ASTNode getThen();
      const ASTNode getThen() const;

      void setElse(ASTNode node);
      ASTNode getElse();
      const ASTNode getElse() const;
      bool hasElse() const;

      void setIfHeaderEndLoc(SourceLoc sloc);
      SourceRange getIfHeaderRange() const;
      SourceLoc getIfHeaderEndLoc() const;

      static bool classof(const Stmt* stmt) {
        return (stmt->getKind() == StmtKind::ConditionStmt);
      }

    private:
      SourceLoc ifHeadEndLoc_;

      Expr* cond_ = nullptr;
      ASTNode then_, else_;
  };

  // CompoundStmt
  //    A group of statements delimited by curly brackets {}
  class CompoundStmt : public Stmt {
    private:
      using NodeVecTy = std::vector<ASTNode>;

    public:
      CompoundStmt();
      CompoundStmt(SourceRange range);

      void addNode(ASTNode stmt);
      void setNode(ASTNode node, std::size_t idx);

      ASTNode getNode(std::size_t ind);
      const ASTNode getNode(std::size_t ind) const;
      NodeVecTy& getNodes();

      bool isEmpty() const;
      std::size_t size() const;

      NodeVecTy::iterator nodes_begin();
      NodeVecTy::iterator nodes_end();

      NodeVecTy::const_iterator nodes_begin() const;
      NodeVecTy::const_iterator nodes_end() const;

      static bool classof(const Stmt* stmt) {
        return (stmt->getKind() == StmtKind::CompoundStmt);
      }

    private:
      NodeVecTy nodes_;
  };

  // WhileStmt
  //    A while loop
  class WhileStmt : public Stmt {
    public:
      WhileStmt();
      WhileStmt(Expr* cond, ASTNode body, SourceRange range,
        SourceLoc headerEndLoc);

      void setCond(Expr* cond);
      Expr* getCond();
      const Expr* getCond() const;

      void setBody(ASTNode body);
      ASTNode getBody();
      const ASTNode getBody() const;

      SourceLoc getHeaderEndLoc() const;
      SourceRange getHeaderRange() const;

      static bool classof(const Stmt* stmt) {
        return (stmt->getKind() == StmtKind::WhileStmt);
      }

    private:
      SourceLoc headerEndLoc_;
      Expr* cond_ = nullptr;
      ASTNode body_;
  };
}

