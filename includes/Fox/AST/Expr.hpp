//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
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
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/Support/TrailingObjects.h"

namespace fox   {
  // Forward Declarations
  class Identifier;
  class ASTContext;
  class TypeBase;

  /// Enum representing every kind of expression that exist.
  enum class ExprKind: std::uint8_t {
    #define EXPR(ID,PARENT) ID,
    #define EXPR_RANGE(ID, FIRST, LAST) First_##ID = FIRST, Last_##ID = LAST,
    #include "ExprNodes.def"
  };

  // ostream to print kinds
  std::ostream& operator<<(std::ostream& os, ExprKind kind);

  /// Expr
  ///    Common base class for every expression
  class alignas(ExprAlignement) Expr {
    // Delete copy ctor/operator (can cause corruption with trailing objects)
    Expr(const Expr&) = delete;
    Expr& operator=(const Expr&) = delete;
    public:
      ExprKind getKind() const;

      SourceRange getSourceRange() const;
      SourceLoc getBeginLoc() const;
      SourceLoc getEndLoc() const;

      void setType(Type type);
      Type getType() const;

      /// Debug method. Does a ASTDump of this node to std::cerr
      void dump() const;

      // Prohibit the use of the vanilla new/delete
      void *operator new(std::size_t) noexcept = delete;
      void operator delete(void *) noexcept = delete;

    protected:
      // Allow allocation through the ASTContext
      void* operator new(std::size_t sz, ASTContext& ctxt,
        std::uint8_t align = alignof(Expr));

      // Also, allow allocation with a placement new
      // (needed for class using trailing objects)
      void* operator new(std::size_t , void* mem);

      Expr(ExprKind kind);

    private:
      const ExprKind kind_;
      Type type_;
  };

  /// A Vector of Expr*
  using ExprVector = SmallVector<Expr*, 4>;

  /// BinaryExpr
  ///    A binary expression
  class BinaryExpr final : public Expr {
    public:
      enum class OpKind: std::uint8_t {
        #define BINARY_OP(ID, SIGN, NAME) ID,
        #include "Operators.def"
      };

      static BinaryExpr* create(ASTContext& ctxt, OpKind op, 
        Expr* lhs, Expr* rhs, SourceRange opRange);

      void setLHS(Expr* expr);
      Expr* getLHS() const;

      void setRHS(Expr* expr);
      Expr* getRHS() const;

      void setOp(OpKind op);
      OpKind getOp() const;

      /// Checks that the operation isn't OpKind::Invalid
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
      SourceRange getSourceRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::BinaryExpr);
      }

      /// Returns the user-readable sign of this expression's 
      /// operator as a string
      std::string getOpSign() const;

      /// Returns the user-readable name of this expression's
      /// operator as a string.
      std::string getOpName() const;

      /// Returns the user-readble Id (kind) of this expression's
      /// operator as a string.
      std::string getOpID() const;

    private:
      BinaryExpr(OpKind op, Expr* lhs, Expr* rhs, SourceRange opRange);

      SourceRange opRange_;
      Expr* lhs_ = nullptr;
      Expr* rhs_ = nullptr;
      OpKind op_ = OpKind::Invalid;
  };

  /// UnaryExpr
  ///    A prefix unary expression: -2
  class UnaryExpr final : public Expr {
    public: 
      enum class OpKind: std::uint8_t {
        #define UNARY_OP(ID, SIGN, NAME) ID,
        #include "Operators.def"
      };

      static UnaryExpr* create(ASTContext& ctxt, OpKind op, Expr* node,
        SourceRange opRange);
      
      void setExpr(Expr* expr);
      Expr* getExpr() const;

      OpKind getOp() const;
      void setOp(OpKind op);

      SourceRange getOpRange() const;
      SourceRange getSourceRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::UnaryExpr);
      }

      /// Returns the user-readable sign of the operator as a string
      std::string getOpSign() const;

      /// Returns the user-readable name as a string
      std::string getOpName() const;

      /// Returns the user-readble Id (kind) as a string
      std::string getOpID() const;

    private:
      UnaryExpr(OpKind op, Expr* node, SourceRange opRange);

      SourceRange opRange_;
      Expr* expr_ = nullptr;
      OpKind op_ = OpKind::Invalid;
  };

  /// CastExpr
  ///    An explicit "as" cast expression: foo as int
  class CastExpr final : public Expr {
    public:      
      static CastExpr* create(ASTContext& ctxt, TypeLoc castGoal, Expr* expr);

      void setCastTypeLoc(TypeLoc goal);
      TypeLoc getCastTypeLoc() const;

      void setExpr(Expr* expr);
      Expr* getExpr() const;

      SourceRange getSourceRange() const;

      /// Returns true if this cast is considered "useless"
      ///  e.g. "0 as int" is a useless cast.
      bool isUseless() const;
      void markAsUselesss();

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::CastExpr);
      }

    private:
      CastExpr(TypeLoc castGoal, Expr* expr);

      TypeLoc goal_;
      // The child expr + a "isUseless" flag.
      //  The cast is marked as useless when the child
      //  is already of the desired type (e.g. "0 as int")
      llvm::PointerIntPair<Expr*, 1, bool> exprAndIsUseless_;
  };

  /// AnyLiteralExpr
  ///  Common base class for any literal expression.
  class AnyLiteralExpr : public Expr {
    public:
      SourceRange getSourceRange() const;

      static bool classof(const Expr* expr) {
        using EK = ExprKind;
        EK k = expr->getKind();
        return (k >= EK::First_AnyLiteralExpr) 
          && (k <= EK::Last_AnyLiteralExpr);
       }

    protected:
      AnyLiteralExpr(ExprKind kind, SourceRange range);

    private:
      SourceRange range_;
  };

  /// CharLiteralExpr
  ///    A char literal: 'a'
  class CharLiteralExpr : public AnyLiteralExpr {
    public:
      static CharLiteralExpr* create(ASTContext& ctxt,
        FoxChar val, SourceRange range);

      FoxChar getVal() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::CharLiteralExpr);
      }

    private:
      CharLiteralExpr(FoxChar val, SourceRange range);

      const FoxChar val_ = ' ';
  };

  /// IntegerLiteralExpr
  ///    An integer literal: 2
  class IntegerLiteralExpr final : public AnyLiteralExpr {
    public:
      static IntegerLiteralExpr* create(ASTContext& ctxt, 
        FoxInt val, SourceRange range);

      FoxInt getVal() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::IntegerLiteralExpr);
      }

    private:
      IntegerLiteralExpr(FoxInt val, SourceRange range);

      const FoxInt val_ = 0;
  };

  /// DoubleLiteralExpr
  ///    A double literal: 3.14
  class DoubleLiteralExpr final : public AnyLiteralExpr {
    public:
      static DoubleLiteralExpr* create(ASTContext& ctxt, FoxDouble val,
        SourceRange range);

      FoxDouble getVal() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::DoubleLiteralExpr);
      }

    private:
      DoubleLiteralExpr(FoxDouble val, SourceRange range);

      const FoxDouble val_ = 0.0;
  };

  /// StringLiteralExpr
  ///    A string literal: "foo"
  class StringLiteralExpr final : public AnyLiteralExpr {
    public:
      static StringLiteralExpr* create(ASTContext& ctxt, string_view val,
        SourceRange range);

      string_view getVal() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::StringLiteralExpr);
      }

    private:
      StringLiteralExpr(string_view val, SourceRange range);

      const string_view val_ = "";
  };

  /// BoolLiteralExpr
  ///    true/false boolean literal
  class BoolLiteralExpr final : public AnyLiteralExpr {
    public:
      static BoolLiteralExpr* create(ASTContext& ctxt, bool val, 
        SourceRange range);

      bool getVal() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::BoolLiteralExpr);
      }

    private:
      BoolLiteralExpr(bool val, SourceRange range);

      const bool val_ : 1;
  };

  /// ArrayLiteralExpr
  ///    An array literal: [1, 2, 3]
  class ArrayLiteralExpr final : public AnyLiteralExpr, 
    llvm::TrailingObjects<ArrayLiteralExpr, Expr*> {
    friend TrailingObjects;
    public:
      using SizeTy = std::uint16_t;
      static constexpr auto maxElems = std::numeric_limits<SizeTy>::max();

      static ArrayLiteralExpr* create(ASTContext& ctxt, ArrayRef<Expr*> elems,
        SourceRange range);

      MutableArrayRef<Expr*> getExprs();
      ArrayRef<Expr*> getExprs() const;
      Expr* getExpr(std::size_t idx) const;
      void setExpr(Expr* expr, std::size_t idx);
      std::size_t numElems() const;
      bool isEmpty() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::ArrayLiteralExpr);
      }

    private:
      ArrayLiteralExpr(ArrayRef<Expr*> elems, SourceRange range);

      const SizeTy numElems_;
  };

  /// UnresolvedExpr
  ///    A small common base class for unresolved expressions
  class UnresolvedExpr : public Expr {
    public:
      static bool classof(const Expr* expr) {
        using EK = ExprKind;
        EK k = expr->getKind();
        return (k >= EK::First_UnresolvedExpr) 
          && (k <= EK::Last_UnresolvedExpr);
      }

    protected:
      UnresolvedExpr(ExprKind kind);
  };

  /// UnresolvedDeclRefExpr
  ///    Represents a unresolved DeclRef
  class UnresolvedDeclRefExpr final : public UnresolvedExpr {
    public:
      static UnresolvedDeclRefExpr* create(ASTContext& ctxt, Identifier id,
        SourceRange range);

      void setIdentifier(Identifier id);
      Identifier getIdentifier() const;

      SourceRange getSourceRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::UnresolvedDeclRefExpr);
      }
    private:
      UnresolvedDeclRefExpr(Identifier id, SourceRange range);

      SourceRange range_;
      Identifier id_;
  };

  /// DeclRefExpr
  ///    A resolved reference to a ValueDecl. 
  class DeclRefExpr final : public Expr {
    public:
      static DeclRefExpr* create(ASTContext& ctxt, ValueDecl* decl,
        SourceRange range);

      ValueDecl* getDecl() const;
      void setDecl(ValueDecl* decl);

      SourceRange getSourceRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::DeclRefExpr);
      }

    private:
      DeclRefExpr(ValueDecl* decl, SourceRange range);

      SourceRange range_;
      ValueDecl* decl_ = nullptr;
  };

  /// MemberOfExpr
  ///    A member access : foo.bar
  class MemberOfExpr final : public Expr {
    public:
      static MemberOfExpr* create(ASTContext& ctxt, Expr* base, 
        Identifier membID, SourceRange membIDRange, SourceLoc dotLoc);

      void setExpr(Expr* expr);
      Expr* getExpr() const;

      void setMemberID(Identifier id);
      Identifier getMemberID() const;
      SourceRange getMemberIDRange() const;
      SourceLoc getDotLoc() const;

      SourceRange getSourceRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::MemberOfExpr);
      }

    private:
      MemberOfExpr(Expr* base, Identifier membID, 
				SourceRange range, SourceLoc dotLoc);

      SourceLoc dotLoc_;
      SourceRange membIDRange_;
      Expr* base_ = nullptr;
      Identifier memb_;
  };

  /// ArraySubscriptExpr
  ///    Array access (or subscript): foo[3]
  class ArraySubscriptExpr final : public Expr {
    public:
      static ArraySubscriptExpr* create(ASTContext& ctxt, Expr* base, 
        Expr* idx, SourceLoc rightSqBrLoc);
      
      void setBase(Expr* expr);
      Expr* getBase() const;

      void setIndex(Expr* expr);
      Expr* getIndex() const;

      SourceRange getSourceRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::ArraySubscriptExpr);
      }

    private:
      ArraySubscriptExpr(Expr* base, Expr* idx, SourceLoc rightSqBrLoc);

      SourceLoc rightSqBrLoc_;
      Expr* base_ = nullptr;
      Expr* idxExpr_ = nullptr;
  };

  /// CallExpr
  ///    A function call: foo(3.14)
  class CallExpr final : public Expr, 
    llvm::TrailingObjects<CallExpr, Expr*> {
    friend class TrailingObjects;
    public:    
      using SizeTy = std::uint8_t;
      static constexpr auto maxArgs = std::numeric_limits<SizeTy>::max();

      static CallExpr* create(ASTContext& ctxt, Expr* callee,
        ArrayRef<Expr*> args, SourceLoc rRoBrLoc);

      void setCallee(Expr* base);
      Expr* getCallee() const;

      SizeTy numArgs() const;
      MutableArrayRef<Expr*> getArgs();
      ArrayRef<Expr*> getArgs() const;
      Expr* getArg(std::size_t idx) const;
      void setArg(Expr* arg, std::size_t idx);

      /// Returns a SourceRange that covers every argument passed
      /// to the Call. Returns an invalid SourceRange if numArgs() == 0.
      SourceRange getArgsRange() const;

      SourceRange getSourceRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::CallExpr);
      }

    private:
      CallExpr(Expr* callee, ArrayRef<Expr*> args, SourceLoc rRoBrLoc);

      SourceLoc rightRoBrLoc_;
      const SizeTy numArgs_ = 0;
      Expr* callee_ = nullptr;
  };

  /// ErrorExpr
  ///   Represents a failed expr that couldn't be resolved.
  ///   This expression always has an ErrorType, and has no
  ///   source location information.
  class ErrorExpr final : public Expr {
    public:
      // Creates an ErrorExpr with ErrorType as the type.
      static ErrorExpr* create(ASTContext& ctxt);

      // Returns SourceRange()
      SourceRange getSourceRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::ErrorExpr);
      }

    private:
      ErrorExpr();
  };
}

