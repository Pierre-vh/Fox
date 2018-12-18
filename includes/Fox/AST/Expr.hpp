//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Expr.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the Expr hierarchy.
//----------------------------------------------------------------------------//

#pragma once

#include "ASTAligns.hpp"
#include "Fox/Common/Typedefs.hpp"
#include "Fox/AST/Type.hpp"
#include "Fox/AST/Identifier.hpp"
#include <vector>

namespace fox   {
  // Forward Declarations
  class Identifier;
  class ASTContext;
  class TypeBase;

  // This enum represents every possible Expression subclass.
  // It is automatically generated using Fox/AST/ExprNodes.def
  enum class ExprKind: std::uint8_t {
    #define EXPR(ID,PARENT) ID,
    #define EXPR_RANGE(ID, FIRST, LAST) First_##ID = FIRST, Last_##ID = LAST,
    #include "ExprNodes.def"
  };

  // ostream to print kinds
  std::ostream& operator<<(std::ostream& os, ExprKind kind);

  // Expr
  //    Common base class for every expression
  class alignas(ExprAlignement) Expr {
    public:
      ExprKind getKind() const;

      void setRange(SourceRange range);
      SourceRange getRange() const;
      SourceLoc getBegin() const;
      SourceLoc getEnd() const;

      void setType(Type type);
      Type getType() const;

      // Debug method. Does a ASTDump of this node to std::cerr
      void dump() const;

    protected:
      // Operator new/delete overloads : They're protected as they should
      // only be used through ::create

      // Prohibit the use of builtin placement new & delete
      void *operator new(std::size_t) throw() = delete;
      void operator delete(void *) throw() = delete;
      void* operator new(std::size_t, void*) = delete;

      // Only allow allocation through the ASTContext
      void* operator new(std::size_t sz, ASTContext &ctxt,
        std::uint8_t align = alignof(Expr));

      // Companion operator delete to silence C4291 on MSVC
      void operator delete(void*, ASTContext&, std::uint8_t) {}

      Expr(ExprKind kind, SourceRange range);

    private:
      const ExprKind kind_;
      Type type_;
      SourceRange range_;
  };

  // A Vector of Pointers to Expressions
  using ExprVector = std::vector<Expr*>;

  // BinaryExpr
  //    A binary expression
  class BinaryExpr final : public Expr {
    public:
      enum class OpKind: std::uint8_t {
        #define BINARY_OP(ID, SIGN, NAME) ID,
        #include "Operators.def"
      };

      static BinaryExpr* create(ASTContext& ctxt, OpKind op, 
        Expr* lhs, Expr* rhs, SourceRange range, SourceRange opRange);

      void setLHS(Expr* expr);
      Expr* getLHS() const;

      void setRHS(Expr* expr);
      Expr* getRHS() const;

      void setOp(OpKind op);
      OpKind getOp() const;

      // Checks that the operation isn't OpKind::Invalid
      bool isValidOp() const;
      // + (only concat)
      bool isConcat() const;
      // + or - (no concatenation)
      bool isAdditive() const;
      // *, % or /
      bool isMultiplicative() const;
      // **
      bool isExponent() const;
      // =
      bool isAssignement() const;
      // ==, !=, >=, <=, > or <
      bool isComparison() const;
      // >=, <=, >, <
      bool isRankingComparison() const;
      // && or ||
      bool isLogical() const;

      SourceRange getOpRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::BinaryExpr);
      }

      // Returns the user-readable sign of this expression's 
      // operator as a string
      std::string getOpSign() const;

      // Returns the user-readable name of this expression's
      // operator as a string.
      std::string getOpName() const;

      // Returns the user-readble Id (kind) of this expression's
      // operator as a string.
      std::string getOpID() const;

    private:
      BinaryExpr(OpKind op, Expr* lhs, Expr* rhs,  SourceRange range,
        SourceRange opRange);

      SourceRange opRange_;
      Expr* lhs_ = nullptr;
      Expr* rhs_ = nullptr;
      OpKind op_ = OpKind::Invalid;
  };

  // UnaryExpr
  //    A unary expression: -2
  class UnaryExpr final : public Expr {
    public: 
      enum class OpKind: std::uint8_t {
        #define UNARY_OP(ID, SIGN, NAME) ID,
        #include "Operators.def"
      };

      static UnaryExpr* create(ASTContext& ctxt, OpKind op, Expr* node,
        SourceRange range, SourceRange opRange);
      
      void setExpr(Expr* expr);
      Expr* getExpr() const;

      OpKind getOp() const;
      void setOp(OpKind op);

      SourceRange getOpRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::UnaryExpr);
      }

      // Returns the user-readable sign of the operator as a string
      std::string getOpSign() const;

      // Returns the user-readable name as a string
      std::string getOpName() const;

      // Returns the user-readble Id (kind) as a string
      std::string getOpID() const;

    private:
      UnaryExpr(OpKind op, Expr* node, SourceRange range,
        SourceRange opRange);

      SourceRange opRange_;
      Expr* expr_ = nullptr;
      OpKind op_ = OpKind::Invalid;
  };

  // CastExpr
  //    An explicit "as" cast expression: foo as int
  class CastExpr final : public Expr {
    public:      
      static CastExpr* create(ASTContext& ctxt, TypeLoc castGoal,
        Expr* expr, SourceRange range);

      void setCastTypeLoc(TypeLoc goal);
      TypeLoc getCastTypeLoc() const;
      Type getCastType() const;
      SourceRange getCastRange() const;

      void setExpr(Expr* expr);
      Expr* getExpr() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::CastExpr);
      }

    private:
      CastExpr(TypeLoc castGoal, Expr* expr, SourceRange range);

      TypeLoc goal_;
      Expr* expr_ = nullptr;
  };

  // CharLiteralExpr
  //    A char literal: 'a'
  class CharLiteralExpr : public Expr {
    public:
      static CharLiteralExpr* create(ASTContext& ctxt,
        FoxChar val, SourceRange range);

      FoxChar getVal() const;
      void setVal(FoxChar val);

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::CharLiteralExpr);
      }

    private:
      CharLiteralExpr(FoxChar val, SourceRange range);

      FoxChar val_ = ' ';
  };

  // IntegerLiteralExpr
  //    An integer literal: 2
  class IntegerLiteralExpr final : public Expr {
    public:
      static IntegerLiteralExpr* create(ASTContext& ctxt, 
        FoxInt val, SourceRange range);

      FoxInt getVal() const;
      void setVal(FoxInt val);

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::IntegerLiteralExpr);
      }

    private:
      IntegerLiteralExpr(FoxInt val, SourceRange range);

      FoxInt val_ = 0;
  };

  // FloatLiteralExpr
  //    A float literal: 3.14
  class FloatLiteralExpr final : public Expr {
    public:
      static FloatLiteralExpr* create(ASTContext& ctxt, FoxFloat val,
        SourceRange range);

      FoxFloat getVal() const;
      void setVal(FoxFloat val);

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::FloatLiteralExpr);
      }

    private:
      FloatLiteralExpr(FoxFloat val, SourceRange range);

      FoxFloat val_ = 0.0;
  };

  // StringLiteralExpr
  //    A string literal: "foo"
  class StringLiteralExpr final : public Expr {
    public:
      static StringLiteralExpr* create(ASTContext& ctxt, const FoxString& val,
        SourceRange range);

      std::string getVal() const;
      void setVal(const FoxString& val);

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::StringLiteralExpr);
      }

    private:
      StringLiteralExpr(const FoxString& val, SourceRange range);

      FoxString val_ = "";
  };

  // BoolLiteralExpr
  //    true/false boolean literal
  class BoolLiteralExpr final : public Expr {
    public:
      static BoolLiteralExpr* create(ASTContext& ctxt, FoxBool val, 
        SourceRange range);

      bool getVal() const;
      void setVal(FoxBool val);

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::BoolLiteralExpr);
      }

    private:
      BoolLiteralExpr(FoxBool val, SourceRange range);

      FoxBool val_ = false;
  };

  // ArrayLiteralExpr
  //    An array literal: [1, 2, 3]
  class ArrayLiteralExpr final : public Expr {
    public:
      static ArrayLiteralExpr* create(ASTContext& ctxt, 
        const ExprVector& exprs, SourceRange range);

      ExprVector& getExprs();
      Expr* getExpr(std::size_t idx) const;

      void setExprs(ExprVector&& elist);
      void setExpr(Expr* expr, std::size_t idx);

      std::size_t getSize() const;
      bool isEmpty() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::ArrayLiteralExpr);
      }

    private:
      ArrayLiteralExpr(const ExprVector& exprs, SourceRange range);

      ExprVector exprs_;
  };

  // UnresolvedExpr
  //    A small common base class for unresolved expressions
  class UnresolvedExpr : public Expr {
    public:
      static bool classof(const Expr* expr) {
        using EK = ExprKind;
        EK k = expr->getKind();
        return (k >= EK::First_UnresolvedExpr) 
          && (k <= EK::Last_UnresolvedExpr);
      }

    protected:
      UnresolvedExpr(ExprKind kind, SourceRange range);
  };

  // UnresolvedDeclRefExpr
  //    Represents a unresolved DeclRef
  class UnresolvedDeclRefExpr final : public UnresolvedExpr {
    public:
      static UnresolvedDeclRefExpr* create(ASTContext& ctxt, Identifier id,
        SourceRange range);

      void setIdentifier(Identifier id);
      Identifier getIdentifier() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::UnresolvedDeclRefExpr);
      }
    private:
      UnresolvedDeclRefExpr(Identifier id, SourceRange range);

      Identifier id_;
  };

  // DeclRefExpr
  //    A resolved reference to a ValueDecl. 
  class DeclRefExpr final : public Expr {
    public:
      static DeclRefExpr* create(ASTContext& ctxt, ValueDecl* decl,
        SourceRange range);

      ValueDecl* getDecl() const;
      void setDecl(ValueDecl* decl);

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::DeclRefExpr);
      }

    private:
      DeclRefExpr(ValueDecl* decl, SourceRange range);

      ValueDecl * decl_ = nullptr;
  };

  // MemberOfExpr
  //    A member access : foo.bar
  class MemberOfExpr final : public Expr {
    public:
      static MemberOfExpr* create(ASTContext& ctxt, Expr* base, 
        Identifier membID, SourceRange range, SourceLoc dotLoc);

      void setExpr(Expr* expr);
      Expr* getExpr() const;

      void setMemberID(Identifier id);
      Identifier getMemberID() const;

      SourceLoc getDotLoc() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::MemberOfExpr);
      }

    private:
      MemberOfExpr(Expr* base, Identifier membID, 
				SourceRange range, SourceLoc dotLoc);

      SourceLoc dotLoc_;
      Expr* base_ = nullptr;
      Identifier memb_;
  };

  // ArraySubscriptExpr
  //    Array access (or subscript): foo[3]
  class ArraySubscriptExpr final : public Expr {
    public:
      static ArraySubscriptExpr* create(ASTContext& ctxt, Expr* base, 
        Expr* idx, SourceRange range);
      
      void setBase(Expr* expr);
      Expr* getBase() const;

      void setIndex(Expr* expr);
      Expr* getIndex() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::ArraySubscriptExpr);
      }

    private:
      ArraySubscriptExpr(Expr* base, Expr* idx, SourceRange range);

      Expr* base_ = nullptr;
      Expr* idxExpr_ = nullptr;
  };

  // FunctionCallExpr
  //    A function call: foo(3.14)
  class FunctionCallExpr final : public Expr {
    public:    
      static FunctionCallExpr* create(ASTContext &ctxt, Expr* callee,
        const ExprVector& args, SourceRange range);

      void setCallee(Expr* base);
      Expr* getCallee() const;

      ExprVector& getArgs();
      Expr* getArg(std::size_t idx) const;

      void setArgs(ExprVector&& exprs);
      void setArg(Expr* arg, std::size_t idx);

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::FunctionCallExpr);
      }

    private:
      FunctionCallExpr(Expr* callee, const ExprVector& args, SourceRange range);

      Expr* callee_ = nullptr;
      ExprVector args_;
  };
}

