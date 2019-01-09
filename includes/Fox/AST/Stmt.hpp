//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Stmt.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the "Stmt" hierarchy          
//----------------------------------------------------------------------------//

#pragma once

#include "ASTAligns.hpp"
#include "ASTNode.hpp"
#include "Fox/Common/Source.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/TrailingObjects.h"

namespace fox {
  /// Enum representing every kind of statement that exist.
  enum class StmtKind : std::uint8_t {
    #define STMT(ID,PARENT) ID,
    #define STMT_RANGE(ID,FIRST,LAST) First_##ID = FIRST, Last_##ID = LAST,
    #include "StmtNodes.def"
  };

  // Forward Declarations
  class Decl;
  class Expr;
  class ASTContext;

  /// Stmt
  ///    Common base for every statement
  class alignas(StmtAlignement) Stmt {
    public:
      /// Returns the kind of statement this is
      StmtKind getKind() const;

      SourceRange getRange() const;
      SourceLoc getBegin() const;
      SourceLoc getEnd() const;

      void dump() const;

    protected:
      Stmt(StmtKind kind);

      // Allow allocation through the ASTContext
      void* operator new(std::size_t sz, ASTContext &ctxt,
        std::uint8_t align = alignof(Stmt));

      // Prohibit the use of the vanilla new/delete
      void *operator new(std::size_t) throw() = delete;
      void operator delete(void *) throw() = delete;

      // Also, allow allocation with a placement new
      // (needed for class using trailing objects)
      void* operator new(std::size_t , void* mem);

    private:
      const StmtKind kind_;
  };

  /// NullStmt
  ///    A null statement ';'
  ///    Often used as the body of a condition/loop
  class NullStmt final : public Stmt {
    public:
      /// Creates a NullStmt.
      /// \param ctxt The ASTContext in which memory will be allocated
      /// \param semiLoc The SourceLoc at the semicolon
      static NullStmt* create(ASTContext& ctxt, SourceLoc semiLoc);

      SourceLoc getSemiLoc() const;

      SourceRange getRange() const;

      static bool classof(const Stmt* stmt) {
        return (stmt->getKind() == StmtKind::NullStmt);
      }

    private:
      NullStmt(SourceLoc semiLoc);

      SourceLoc semiLoc_;
  };

  /// ReturnStmt
  ///    A return statement
  class ReturnStmt final : public Stmt {
    public:
      /// Creates a ReturnStmt.
      /// \param ctxt The ASTContext in which memory will be allocated
      /// \param rtr The return expression, if there's one
      /// \param range The full range of the statement (from the beginning
      ///        of the 'return' keyword to the semicolon)
      static ReturnStmt* create(ASTContext& ctxt, Expr* rtr, 
        SourceRange range);

      void setExpr(Expr* e);
      Expr* getExpr() const;
      bool hasExpr() const;

      SourceRange getRange() const;

      static bool classof(const Stmt* stmt) {
        return (stmt->getKind() == StmtKind::ReturnStmt);
      }

    private:
      ReturnStmt(Expr* rtr, SourceRange range);

      SourceRange range_;
      Expr* expr_ = nullptr;
  };

  /// ConditionStmt
  ///    if-then-else conditional statement
  class ConditionStmt final : public Stmt {
    public:
      /// Creates a ReturnStmt.
      /// \param ctxt The ASTContext in which memory will be allocated
      /// \param ifBegLoc The SourceLoc at the beginning of the "if" keyword
      /// \param cond The condition Expr
      /// \param then The then node
      /// \param condElse The else node, if there is one.
      static ConditionStmt* create(ASTContext& ctxt, SourceLoc ifBegLoc,
        Expr* cond, ASTNode then, ASTNode condElse);

      void setCond(Expr* expr);
      Expr* getCond() const;

      void setThen(ASTNode node);
      ASTNode getThen() const;

      void setElse(ASTNode node);
      ASTNode getElse() const;
      bool hasElse() const;

      SourceRange getRange() const;

      static bool classof(const Stmt* stmt) {
        return (stmt->getKind() == StmtKind::ConditionStmt);
      }

    private:
      ConditionStmt(SourceLoc ifBegLoc, Expr* cond, ASTNode then, 
        ASTNode elsenode);

      SourceLoc ifBegLoc_;
      Expr* cond_ = nullptr;
      ASTNode then_, else_;
  };

  /// CompoundStmt
  ///    A group of statements delimited by curly brackets {}
  class CompoundStmt final : public Stmt, 
    llvm::TrailingObjects<CompoundStmt, ASTNode> {
    friend TrailingObjects;
    public:
      using SizeTy = std::uint16_t;

      /// The maximum number of statements that can be contained inside
      /// a CompoundStmt
      static constexpr auto maxNodes = std::numeric_limits<SizeTy>::max();

      /// Creates a CompoundStmt
      /// \param ctxt The ASTContext in which memory will be allocated
      /// \param elem The nodes contained inside this CompoundStmt
      /// \param bracesRange The SourceRange of the CompoundStmt
      ///        (from the left brace to the right brace)
      static CompoundStmt* create(ASTContext& ctxt, ArrayRef<ASTNode> elems,
        SourceRange bracesRange);

      /// Replaces a single node in the CompoundStmt
      void setNode(ASTNode node, std::size_t idx);
      ASTNode getNode(std::size_t ind) const;
      ArrayRef<ASTNode> getNodes() const;
      MutableArrayRef<ASTNode> getNodes();
      std::size_t getSize() const;
      bool isEmpty() const;

      SourceRange getRange() const;

      static bool classof(const Stmt* stmt) {
        return (stmt->getKind() == StmtKind::CompoundStmt);
      }

    private:
      CompoundStmt(ArrayRef<ASTNode> elems, SourceRange bracesRange);

      SourceRange bracesRange_;
      const SizeTy numNodes_;
  };

  /// WhileStmt
  ///   A while loop
  class WhileStmt final : public Stmt {
    public:
      /// Creates a WhileStmt
      /// \param ctxt The ASTContext in which memory will be allocated
      /// \param whBegLoc The SourceLoc at the beginning of the "while" keyword
      /// \param cond The condition Expr
      /// \param then The loop's body
      static WhileStmt* 
      create(ASTContext& ctxt, SourceLoc whBegLoc, Expr* cond, ASTNode body);

      void setCond(Expr* cond);
      Expr* getCond() const;

      void setBody(ASTNode body);
      ASTNode getBody() const;

      SourceRange getRange() const;

      static bool classof(const Stmt* stmt) {
        return (stmt->getKind() == StmtKind::WhileStmt);
      }

    private:
      WhileStmt(SourceLoc whBegLoc, Expr* cond, ASTNode body);

      SourceLoc whBegLoc_;
      Expr* cond_;
      ASTNode body_;
  };
}

