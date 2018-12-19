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
#include "ASTNode.hpp"
#include "Fox/Common/Source.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"

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
  class alignas(StmtAlignement) Stmt {
    public:
      StmtKind getKind() const;

      void setRange(SourceRange range);
      SourceRange getRange() const;
      SourceLoc getBegin() const;
      SourceLoc getEnd() const;

      void dump() const;

    protected:
      Stmt(StmtKind kind, SourceRange range);

      // Operator new/delete overloads : They're protected as they should
      // only be used through ::create

      // Prohibit the use of builtin placement new & delete
      void *operator new(std::size_t) throw() = delete;
      void operator delete(void *) throw() = delete;
      void* operator new(std::size_t, void*) = delete;

      // Only allow allocation through the ASTContext
      void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(Stmt));

      // Companion operator delete to silence C4291 on MSVC
      void operator delete(void*, ASTContext&, std::uint8_t) {}

    private:
      SourceRange range_;
      const StmtKind kind_;
  };

  // NullStmt
  //    A null statement ';'
  //    Often used as the body of a condition/loop
  class NullStmt final : public Stmt {
    public:
      static NullStmt* create(ASTContext& ctxt, 
        SourceLoc semiLoc = SourceLoc());

      void setSemiLoc(SourceLoc semiLoc);
      SourceLoc getSemiLoc() const;

      static bool classof(const Stmt* stmt) {
        return (stmt->getKind() == StmtKind::NullStmt);
      }
    private:
      NullStmt(SourceLoc semiLoc);
  };

  // ReturnStmt
  //    A return statement
  class ReturnStmt final : public Stmt {
    public:
      static ReturnStmt* create(ASTContext& ctxt, Expr* rtr, 
        SourceRange range);

      void setExpr(Expr* e);
      Expr* getExpr() const;
      bool hasExpr() const;

      static bool classof(const Stmt* stmt) {
        return (stmt->getKind() == StmtKind::ReturnStmt);
      }

    private:
      ReturnStmt(Expr* rtr, SourceRange range);

      Expr* expr_ = nullptr;
  };

  // ConditionStmt
  //    if-then-else conditional statement
  class ConditionStmt final : public Stmt {
    public:
      static ConditionStmt* create(ASTContext& ctxt, Expr* cond, ASTNode then,
        ASTNode condElse, SourceRange range, SourceLoc ifHeaderEnd);

      bool isValid() const;

      void setCond(Expr* expr);
      Expr* getCond() const;

      void setThen(ASTNode node);
      ASTNode getThen() const;

      void setElse(ASTNode node);
      ASTNode getElse() const;
      bool hasElse() const;

      void setIfHeaderEndLoc(SourceLoc sloc);
      SourceRange getIfHeaderRange() const;
      SourceLoc getIfHeaderEndLoc() const;

      static bool classof(const Stmt* stmt) {
        return (stmt->getKind() == StmtKind::ConditionStmt);
      }

    private:
      ConditionStmt(Expr* cond, ASTNode then, ASTNode elsenode,
        SourceRange range, SourceLoc ifHeaderEndLoc);

      SourceLoc ifHeadEndLoc_;
      Expr* cond_ = nullptr;
      ASTNode then_, else_;
  };

  // CompoundStmt
  //    A group of statements delimited by curly brackets {}
  class CompoundStmt final : public Stmt {
    public:
      // The type of the vector used by CompoundStmt internally to 
      // store the ASTNodes
      using NodeVec = SmallVector<ASTNode, 4>;

      // range argument is optional because when parsing CompoundStmts, we
      // don't know the full range of the Stmt until we finished parsing it.
      // Usually we create the CompoundStmt first, then fill it with the Stmts
      // as we parse them, and then set the range before returning, once
      // the } has been parsed.
      static CompoundStmt* create(ASTContext& ctxt, 
        SourceRange range = SourceRange());

      void addNode(ASTNode stmt);
      void setNode(ASTNode node, std::size_t idx);

      ASTNode getNode(std::size_t ind) const;
      NodeVec& getNodes();
      const NodeVec& getNodes() const;

      bool isEmpty() const;
      std::size_t size() const;

      static bool classof(const Stmt* stmt) {
        return (stmt->getKind() == StmtKind::CompoundStmt);
      }

    private:
      CompoundStmt(SourceRange range);

      NodeVec nodes_;
  };

  // WhileStmt
  //    A while loop
  class WhileStmt final : public Stmt {
    public:
      static WhileStmt* create(ASTContext& ctxt, Expr* cond, ASTNode body,
        SourceRange range, SourceLoc headerEnd);

      void setCond(Expr* cond);
      Expr* getCond() const;

      void setBody(ASTNode body);
      ASTNode getBody() const;

      SourceLoc getHeaderEndLoc() const;
      SourceRange getHeaderRange() const;

      static bool classof(const Stmt* stmt) {
        return (stmt->getKind() == StmtKind::WhileStmt);
      }

    private:
      WhileStmt(Expr* cond, ASTNode body, SourceRange range,
        SourceLoc headerEndLoc);

      SourceLoc headerEndLoc_;
      Expr* cond_ = nullptr;
      ASTNode body_;
  };
}

