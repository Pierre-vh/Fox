//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Expr.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the Expr Hierarchy.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/Common/Typedefs.hpp"
#include "Fox/AST/Type.hpp"
#include <vector>

namespace fox   {
  // Kinds of Expressions
  enum class ExprKind: std::uint8_t {
    // TODO: if EXPR_RANGE is added, support it in operator<<
    #define EXPR(ID,PARENT) ID,
    #include "ExprNodes.def"
  };

  // ostream to print kinds
  std::ostream& operator<<(std::ostream& os, ExprKind kind);

  // Forward Declarations
  class Identifier;
  class ASTContext;
  class TypeBase;

  // Expr
  //    Common base class for every expression
  class Expr {
    public:
      ExprKind getKind() const;

      void setRange(SourceRange range);
      SourceRange getRange() const;

      void setType(Type type);
      Type& getType();
      const Type getType() const;

      // Dumps the structure of this expr to cerr.
      void dump() const;

      // Prohibit the use of builtin placement new & delete
      void *operator new(std::size_t) throw() = delete;
      void operator delete(void *) throw() = delete;
      void* operator new(std::size_t, void*) = delete;

      // Only allow allocation through the ASTContext
      void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(Expr));

      // Companion operator delete to silence C4291 on MSVC
      void operator delete(void*, ASTContext&, std::uint8_t) {}

    protected:
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
  class BinaryExpr : public Expr {
    public:
      enum class OpKind: std::uint8_t {
        #define BINARY_OP(ID, SIGN, NAME) ID,
        #include "Operators.def"
      };

      BinaryExpr();
      BinaryExpr(OpKind op, Expr* lhs, Expr* rhs, 
        SourceRange range, SourceRange opRange);

      void setLHS(Expr* expr);
      Expr* getLHS();
      const Expr* getLHS() const;

      void setRHS(Expr* expr);
      Expr* getRHS();
      const Expr* getRHS() const;

      OpKind getOp() const;
      void setOp(OpKind op);

      SourceRange getOpRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::BinaryExpr);
      }

      // Get information about an operator as a string
      static std::string getOpSign(OpKind op);
      static std::string getOpName(OpKind op);
      static std::string getOpID(OpKind op);

    private:
      SourceRange opRange_;
      Expr* lhs_ = nullptr;
      Expr* rhs_ = nullptr;
      OpKind op_ = OpKind::Invalid;
  };

  // UnaryExpr
  //    A unary expression: -2
  class UnaryExpr : public Expr {
    public: 
      enum class OpKind: std::uint8_t {
        #define UNARY_OP(ID, SIGN, NAME) ID,
        #include "Operators.def"
      };

      UnaryExpr();
      UnaryExpr(OpKind op, Expr* node, SourceRange range,
        SourceRange opRange);
      
      void setExpr(Expr* expr);
      Expr* getExpr();
      const Expr* getExpr() const;

      OpKind getOp() const;
      void setOp(OpKind op);

      SourceRange getOpRange() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::UnaryExpr);
      }

      // Get information about an operator as a string
      static std::string getOpSign(OpKind op);
      static std::string getOpName(OpKind op);
      static std::string getOpID(OpKind op);

      // Non static variants to get info on this instance
      std::string getOpSign() const;
      std::string getOpName() const;
      std::string getOpID() const;

    private:
      SourceRange opRange_;
      Expr* expr_ = nullptr;
      OpKind op_ = OpKind::Invalid;
  };

  // CastExpr
  //    An explicit "as" cast expression: foo as int
  class CastExpr : public Expr {
    public:
      CastExpr();
      CastExpr(TypeLoc castGoal, Expr* expr, SourceRange range);
      
      void setCastTypeLoc(TypeLoc goal);
      TypeLoc& getCastTypeLoc();
      const TypeLoc getCastTypeLoc() const;

      void setExpr(Expr* expr);
      Expr* getExpr();
      const Expr* getExpr() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::CastExpr);
      }

    private:
      TypeLoc goal_;
      Expr* expr_ = nullptr;
  };

  // CharLiteralExpr
  //    A char literal: 'a'
  class CharLiteralExpr : public Expr {
    public:
      CharLiteralExpr();
      CharLiteralExpr(FoxChar val, SourceRange range);

      FoxChar getVal() const;
      void setVal(FoxChar val);

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::CharLiteralExpr);
      }

    private:
      FoxChar val_ = ' ';
  };

  // IntegerLiteralExpr
  //    An integer literal: 2
  class IntegerLiteralExpr : public Expr {
    public:
      IntegerLiteralExpr();
      IntegerLiteralExpr(FoxInt val, SourceRange range);

      FoxInt getVal() const;
      void setVal(FoxInt val);

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::IntegerLiteralExpr);
      }

    private:
      FoxInt val_ = 0;
  };

  // FloatLiteralExpr
  //    A float literal: 3.14
  class FloatLiteralExpr : public Expr {
    public:
      FloatLiteralExpr();
      FloatLiteralExpr(FoxFloat val, SourceRange range);

      FoxFloat getVal() const;
      void setVal(FoxFloat val);

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::FloatLiteralExpr);
      }

    private:
      FoxFloat val_ = 0.0f;
  };

  // StringLiteralExpr
  //    A string literal: "foo"
  class StringLiteralExpr : public Expr {
    public:
      StringLiteralExpr();
      StringLiteralExpr(const FoxString& val, SourceRange range);

      std::string getVal() const;
      void setVal(const FoxString& val);

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::StringLiteralExpr);
      }

    private:
      FoxString val_ = "";
  };

  // BoolLiteralExpr
  //    true/false boolean literal
  class BoolLiteralExpr : public Expr {
    public:
      BoolLiteralExpr();
      BoolLiteralExpr(FoxBool val, SourceRange range);

      bool getVal() const;
      void setVal(FoxBool val);

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::BoolLiteralExpr);
      }

    private:
      FoxBool val_ = false;
  };

  // ArrayLiteralExpr
  //    An array literal: [1, 2, 3]
  class ArrayLiteralExpr : public Expr {
    public:
      ArrayLiteralExpr();
      ArrayLiteralExpr(const ExprVector& exprs, SourceRange range);

      ExprVector& getExprs();
      const ExprVector& getExprs() const;
      Expr* getExpr(std::size_t idx);
      const Expr* getExpr(std::size_t idx) const;

      void setExprs(ExprVector&& elist);
      void setExpr(Expr* expr, std::size_t idx);

      std::size_t getSize() const;
      bool isEmpty() const;

      ExprVector::iterator exprs_begin();
      ExprVector::const_iterator exprs_begin() const;

      ExprVector::iterator exprs_end();
      ExprVector::const_iterator exprs_end() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::ArrayLiteralExpr);
      }

    private:
      ExprVector exprs_;
  };

  // DeclRefExpr
  //    A identifier that references a declaration: foo
  class DeclRefExpr : public Expr {
    public:
      DeclRefExpr();
      DeclRefExpr(Identifier* declid, SourceRange range);

      void setIdentifier(Identifier * id);
      Identifier* getIdentifier();
      const Identifier* getIdentifier() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::DeclRefExpr);
      }

    private:
      Identifier* id_ = nullptr;
  };

  // MemberOfExpr
  //    A member access : foo.bar
  class MemberOfExpr : public Expr {
    public:
      MemberOfExpr();
      MemberOfExpr(Expr* base, Identifier *idInfo, 
        SourceRange range, SourceLoc dotLoc);

      void setExpr(Expr* expr);
      Expr* getExpr();
      const Expr* getExpr() const;

      void setMemberID(Identifier* idInfo);
      Identifier* getMemberID();
      const Identifier* getMemberID() const;

      SourceLoc getDotLoc() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::MemberOfExpr);
      }

    private:
      SourceLoc dotLoc_;
      Expr* base_ = nullptr;
      Identifier *membName_ = nullptr;
  };

  // ArraySubscriptExpr
  //    Array access (or subscript): foo[3]
  class ArraySubscriptExpr : public Expr {
    public:
      ArraySubscriptExpr();
      ArraySubscriptExpr(Expr* base, Expr* idx, SourceRange range);

      void setBase(Expr* expr);
      Expr* getBase();
      const Expr* getBase() const;

      void setIndex(Expr* expr);
      Expr* getIndex();
      const Expr* getIndex() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::ArraySubscriptExpr);
      }

    private:
      Expr* base_ = nullptr;
      Expr* idxExpr_ = nullptr;
  };

  // FunctionCallExpr
  //    A function call: foo(3.14)
  class FunctionCallExpr : public Expr {
    public:
      FunctionCallExpr();
      FunctionCallExpr(Expr* callee, const ExprVector& args, SourceRange range);
      
      void setCallee(Expr* base);
      Expr* getCallee();
      const Expr* getCallee() const;

      ExprVector& getArgs();
      const ExprVector& getArgs() const;
      Expr* getArg(std::size_t idx);
      const Expr* getArg(std::size_t idx) const;

      void setArgs(ExprVector&& exprs);
      void setArg(Expr* arg, std::size_t idx);

      ExprVector::iterator args_begin();
      ExprVector::const_iterator args_begin() const;

      ExprVector::iterator args_end();
      ExprVector::const_iterator args_end() const;

      static bool classof(const Expr* expr) {
        return (expr->getKind() == ExprKind::FunctionCallExpr);
      }

    private:
      Expr* callee_ = nullptr;
      ExprVector args_;
  };
}

