//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Sema.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Contains the Sema class, which is used to perform 
// most of the semantic analysis of a Fox AST.
//----------------------------------------------------------------------------//

#pragma once

#include <cstdint>
#include <tuple>
#include "Fox/AST/ASTFwdDecl.hpp"

namespace fox {
  class ASTContext;
  class DiagnosticEngine;
  class Sema {
    public:
      Sema(ASTContext& ctxt, DiagnosticEngine& diags);

      // Other helper classes
      class RAIISetDeclCtxt;

      // Typedefs
      using TypePair = std::pair<Type, Type>;
      using IntegralRankTy = std::uint8_t;

      // Performs semantic analysis on a node and it's children
      //  Typechecks an expression, declaration or statement.
      //
      //  Returns the node that should take this node's place. 
			//	Note that the returned node will always be equal to the argument 
			//	unless the ASTNode contains an Expr. Never returns nullptr.
      ASTNode checkNode(ASTNode node);

      // Performs semantic analysis on an expression and it's children.
      //  Typechecks an expression. 
      //  
      //  Returns the expression or another equivalent expression that 
			//	should replace it. Never nullptr.
      Expr* typecheckExpr(Expr* expr);

      // Return enum for typecheckExprOfType
      //  Ok = the checked expr's type is equivalent to the one requested
      //  NOk = the checked expr's type is not equivalent
      //  Error = the checked expr's type is ErrorType
      enum class CheckedExprResult { Ok, NOk, Error,};

      // Performs semantic analysis on an expression and it's children.
      //  Typechecks an expression whose type is already known.
      //
      //  Returns a pair. The first element of the pair is a success indicator
      //  (seem CheckedExprResult). The second Expr* pointer
      //  is the expression or another equivalent expr that should replace it.
			// \param allowDowncast If set to false, we'll return CheckedExprResult::NOk
			//			  if casting "expr" to "type" results in a downcast.
      std::pair<CheckedExprResult, Expr*> 
      typecheckExprOfType(Expr* expr, Type type, bool allowDowncast = true);

      // Performs semantic analysis on a single statement and it's children.
      void checkStmt(Stmt* stmt);

      // Performs semantic analysis on a single declaration and it's children
      void checkDecl(Decl* decl);

      // The unification algorithms for types of the same subtypes.
      // Tries to make A = B
      //
      // Due to the way Fox's semantics work
      // This unification algorithm won't alter types unless
      // they are CellTypes.
      //
      // Also, this function is commutative.
      bool unify(Type a, Type b);

			// Returns true if the conversion of A to B is a downcast
			//		A and B must both be PrimitiveTypes after a call to Sema::unwrapAll
			//
			//		If A and/or B are not integral types, returns false.
			//		Only returns true if A and B are both integral types
			//		and casting A to B is a downcast.
			//	\param areIntegrals Set to true if both types were integral types
			static bool isDowncast(Type a, Type b, bool* areIntegrals = nullptr);

      // Given 2 types
        // If they are integrals, return the highest ranking integral's type
        // If they are equal, return it's first argument
        // Returns nullptr otherwise.
      // if unwrapTypes is set to true, types are unwrapped together.
      //    e.g. [int] & [int] is unwrapped to 
      //          int & int but [[int]] & [int] is unwrapped to [int] & int
      // if ignoreLValues is set to true, lvalues are ignored prior to 
      // comparison.
      static Type getHighestRankedTy(Type a, Type b,
        bool unwrap = true);

      // This method returns the integral rank that a given type has.
      // type must not be null and must point to a arithmetic type.
      static IntegralRankTy getIntegralRank(Type type);;

      // Given a type, return the Basic type if it can find one, or nullptr.
      // e.g.
      //    LValue(Array(Array(int))) will return int
      static BasicType* findBasicType(Type type);

      // Removes the same number of ArrayType layers on a and b
      static TypePair unwrapArrays(Type a, Type b);

      // Removes all layers of LValue, CellType and ArrayType 
      // until this reaches a point where one (or both) of the
      // types become basic.
      // Note that the result types may not be basic! The function will simply
      // stop unwrapping once one of them becomes basic.
      static TypePair unwrapAll(Type a, Type b);

      DiagnosticEngine& getDiagnosticEngine();
      ASTContext& getASTContext();

      // Sets the current DeclContext and returns a RAII object that will,
      // upon destruction, restore the previous DeclContext.
      RAIISetDeclCtxt setDeclCtxtRAII(DeclContext* dc);
      void setDeclCtxt(DeclContext* dc);
      DeclContext* getDeclCtxt() const;
      bool hasDeclCtxt() const;

    private:
      // Private implementation classes
      class Checker;
      class DeclChecker;
      class StmtChecker;
      class ExprChecker;

      DeclContext* currentDC_ = nullptr;
      
      ASTContext &ctxt_;
      DiagnosticEngine& diags_;
  };

  class Sema::Checker {
    Sema& sema_;
    DiagnosticEngine& diags_;
    ASTContext& ctxt_;
    public:
      Checker(Sema& sema) : sema_(sema),
        diags_(sema.getDiagnosticEngine()), ctxt_(sema.getASTContext()) {}

      ASTContext& getCtxt() { return ctxt_; }
      DiagnosticEngine& getDiags() { return diags_; }
      Sema& getSema() { return sema_; }
  };

  // A Small RAII object that sets the currently active DeclContext
  // for a Sema instance. Upon destruction, it will restore the 
  // Sema's currently active DeclContext to what it was before.
  class Sema::RAIISetDeclCtxt {
      Sema& sema_;
      DeclContext* oldDC_ = nullptr;
    public:
      RAIISetDeclCtxt(Sema& sema, DeclContext* dc) : sema_(sema) {
        oldDC_ = sema.getDeclCtxt();
        sema_.setDeclCtxt(dc);
      }

      ~RAIISetDeclCtxt() {
        sema_.setDeclCtxt(oldDC_);
      }
  };
}
