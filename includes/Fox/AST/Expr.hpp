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
#include "Type.hpp"
#include "Identifier.hpp"
#include "BuiltinTypeMembers.hpp"
#include "Fox/Common/FoxTypes.hpp"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/Support/TrailingObjects.h"

namespace fox   {
  // Forward Declarations
  class ASTContext;

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
      bool isPower() const;
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
        #define LAST_UNARY_OP(ID) last_op = ID
        #include "Operators.def"
      };

      static UnaryExpr* create(ASTContext& ctxt, OpKind op, Expr* child,
        SourceRange opRange);
      
      void setChild(Expr* expr);
      Expr* getChild() const;

      OpKind getOp() const;

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
      UnaryExpr(OpKind op, Expr* child, SourceRange opRange);

      static constexpr std::size_t numOpBits = 3;
      static_assert(static_cast<unsigned>(OpKind::last_op) < (1 << numOpBits),
        "not enough bits in opAndChild_ to represent every unary op kind");

      // The pointer to the child expr + the operation kind
      llvm::PointerIntPair<Expr*, numOpBits, OpKind> opAndChild_;
      SourceRange opRange_;
  };

  /// CastExpr
  ///    An explicit "as" cast expression: foo as int
  class CastExpr final : public Expr {
    public:      
      static CastExpr* create(ASTContext& ctxt, TypeLoc castGoal, Expr* child);

      TypeLoc getCastTypeLoc() const;

      void setChild(Expr* expr);
      Expr* getChild() const;

      SourceRange getSourceRange() const;

      /// Returns true if this cast is considered "useless"
      ///  e.g. "0 as int" is a useless cast.
      bool isUseless() const;
      void markAsUselesss();

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::CastExpr);
      }

    private:
      CastExpr(TypeLoc castGoal, Expr* child);

      TypeLoc goal_;
      // The child expr + a "isUseless" flag.
      //  The cast is marked as useless when the child
      //  is already of the desired type (e.g. "0 as int")
      llvm::PointerIntPair<Expr*, 1, bool> childAndIsUseless_;
  };

  /// AnyLiteralExpr
  ///  Common base class for any literal expression.
  class AnyLiteralExpr : public Expr {
    public:
      SourceRange getSourceRange() const { return range_; }

      static bool classof(const Expr* expr) {
        using EK = ExprKind;
        EK k = expr->getKind();
        return (k >= EK::First_AnyLiteralExpr) 
          && (k <= EK::Last_AnyLiteralExpr);
      }

    protected:
      AnyLiteralExpr(ExprKind kind, SourceRange range) 
        : Expr(kind), range_(range) {}

    private:
      SourceRange range_;
  };

  /// CharLiteralExpr
  ///    A char literal: 'a'
  class CharLiteralExpr : public AnyLiteralExpr {
    public:
      static CharLiteralExpr* create(ASTContext& ctxt,
        FoxChar val, SourceRange range);

      FoxChar getValue() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::CharLiteralExpr);
      }

    private:
      CharLiteralExpr(FoxChar val, SourceRange range);

      const FoxChar val_ = 0;
  };

  /// IntegerLiteralExpr
  ///    An integer literal: 2
  class IntegerLiteralExpr final : public AnyLiteralExpr {
    public:
      static IntegerLiteralExpr* create(ASTContext& ctxt, 
        FoxInt val, SourceRange range);

      FoxInt getValue() const;

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

      FoxDouble getValue() const;

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

      string_view getValue() const;

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

      bool getValue() const;

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
  ///    A small common base for unresolved expressions
  class UnresolvedExpr : public Expr {
    public:
      static bool classof(const Expr* expr) {
        using EK = ExprKind;
        EK k = expr->getKind();
        return (k >= EK::First_UnresolvedExpr) 
          && (k <= EK::Last_UnresolvedExpr);
      }

    protected:
      UnresolvedExpr(ExprKind kind) : Expr(kind) {}
  };

  /// UnresolvedDeclRefExpr
  ///    Represents a unresolved reference to a declaration
  class UnresolvedDeclRefExpr final : public UnresolvedExpr {
    public:
      static UnresolvedDeclRefExpr* create(ASTContext& ctxt, Identifier id,
        SourceRange range);

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

      SourceRange getSourceRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::DeclRefExpr);
      }

    private:
      DeclRefExpr(ValueDecl* decl, SourceRange range);

      SourceRange range_;
      ValueDecl* decl_ = nullptr;
  };

  /// UnresolvedDotExpr
  ///    A unresolved "dot" expr : expr.foo
  class UnresolvedDotExpr final : public UnresolvedExpr {
    public:
      static UnresolvedDotExpr* create(ASTContext& ctxt, Expr* base, 
        Identifier membID, SourceRange membIDRange, SourceLoc dotLoc);

      void setBase(Expr* expr);
      Expr* getBase() const;

      Identifier getMemberIdentifier() const;
      SourceRange getMemberIdentifierRange() const;
      SourceLoc getDotLoc() const;

      SourceRange getSourceRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::UnresolvedDotExpr);
      }

    private:
      UnresolvedDotExpr(Expr* base, Identifier membID, 
				SourceRange range, SourceLoc dotLoc);

      Expr* base_ = nullptr;
      SourceLoc dotLoc_;
      SourceRange membRange_;
      Identifier memb_;
  };

  /// BuiltinMemberRefExpr
  ///   A resolved reference to a builtin member of a type
  ///   (usually a member of a builtin type)
  ///   e.g. "string".size(), array.append(x), etc.
  class BuiltinMemberRefExpr final : public Expr {
    public:
      using BTMKind = BuiltinTypeMemberKind;

      /// Creates a BuiltinMemberRefExpr from a UnresolvedDotExpr
      static BuiltinMemberRefExpr* 
      create(ASTContext& ctxt, UnresolvedDotExpr* expr, BTMKind kind);

      static BuiltinMemberRefExpr* 
      create(ASTContext& ctxt, Expr* base, Identifier membID, 
        SourceRange membIDRange, SourceLoc dotLoc, BTMKind kind);

      void setBase(Expr* expr);
      Expr* getBase() const;

      Identifier getMemberIdentifier() const;
      SourceRange getMemberIdentifierRange() const;
      SourceLoc getDotLoc() const;

      SourceRange getSourceRange() const;

      /// \returns true if this method is called. 
      /// Only available when isMethod() returns true.
      bool isCalled() const;
      /// Marks this method as being called.
      /// Only available when isMethod() returns true.
      void setIsCalled(bool value = true);
      
      /// \returns true if this is a reference to a method.
      bool isMethod() const;
      /// Marks this node as a reference to a method.
      void setIsMethod(bool value = true);

      /// \returns the BuiltinTypeMemberKind
      BuiltinTypeMemberKind getBuiltinTypeMemberKind() const;
      void setBuiltinTypeMemberKind(BuiltinTypeMemberKind value);

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::BuiltinMemberRefExpr);
      }

    private:
      BuiltinMemberRefExpr(Expr* base, Identifier membID, 
        SourceRange membIDRange, SourceLoc dotLoc, BTMKind kind);
       
      // Bit field: 0 bits left
      //  14 bits for the BuiltinTypeMemberKind should be more than enough!
      BTMKind kind_ : 14; 
      bool isCalled_ : 1;
      bool isMethod_ : 1;
      // End bit field
      SourceLoc dotLoc_;
      SourceRange membRange_;
      Expr* base_ = nullptr;
      Identifier memb_;
  };

  /// SubscriptExpr
  ///    Subscript on an Array or a string:
  ///     "hello"[0] or [0, 1, 2][0]
  class SubscriptExpr final : public Expr {
    public:
      static SubscriptExpr* create(ASTContext& ctxt, Expr* base, 
        Expr* idx, SourceLoc rightBracketLoc);
      
      void setBase(Expr* expr);
      Expr* getBase() const;

      void setIndex(Expr* expr);
      Expr* getIndex() const;

      SourceRange getSourceRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::SubscriptExpr);
      }

    private:
      SubscriptExpr(Expr* base, Expr* idx, SourceLoc rightBracketLoc);

      // The loc of the ']'. We only keep this one because it's
      // used to generate the SourceRange of this node, which is calculated
      // from the beginning of the base expression and the loc of the ']'
      // Of course, if I ever need the leftBracket's loc, it's an easy addition.
      SourceLoc rightBracketLoc_;
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
        ArrayRef<Expr*> args, SourceRange parenRange);

      void setCallee(Expr* base);
      Expr* getCallee() const;

      SizeTy numArgs() const;
      MutableArrayRef<Expr*> getArgs();
      ArrayRef<Expr*> getArgs() const;
      Expr* getArg(std::size_t idx) const;
      void setArg(Expr* arg, std::size_t idx);

      /// \returns the range of the call parentheses
      SourceRange getCallParenRange() const;

      SourceRange getSourceRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::CallExpr);
      }

    private:
      CallExpr(Expr* callee, ArrayRef<Expr*> args, SourceRange parenRange);

      SourceRange parenRange_;
      const SizeTy numArgs_ = 0;
      Expr* callee_ = nullptr;
  };

  /// ErrorExpr
  ///   Represents an expression that couldn't be resolved.
  ///
  ///   ErrorExprs are generated during semantic analysis when
  ///   UnresolvedExprs can't be resolved.
  ///
  ///   This expression always has an ErrorType.
  class ErrorExpr final : public Expr {
    public:
      /// Creates an ErrorExpr using \p expr ->getSourceRange()
      /// Automatically sets this node's type to ErrorType.
      static ErrorExpr* create(ASTContext& ctxt, Expr* expr);

      /// Creates an ErrorExpr.
      /// Automatically sets this node's type to ErrorType.
      static ErrorExpr* create(ASTContext& ctxt, SourceRange range);

      SourceRange getSourceRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::ErrorExpr);
      }

    private:
      ErrorExpr(SourceRange range);

      SourceRange range_;
  };
}

