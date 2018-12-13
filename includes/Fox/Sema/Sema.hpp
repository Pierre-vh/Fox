//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Sema.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Contains the Sema class, which is used to perform 
// most of the semantic analysis of a Fox AST.
//----------------------------------------------------------------------------//

// TODO:
//    Once the Semantic Analysis is more or less complete (it can fully
//    check a UnitDecl*), move this class to /lib as an impl detail, and
//    create another file in this folder which will contain the main entry
//    points to begin checking a UnitDecl.
//
//    Once this class becomes "implementation-only", I'll reorganize it a 
//    bit, e.g. by making every method public and making the Checkers non
//    members classes.
//
//    e.g. This file could be named "Requests" or "Check", and contain
//         a single function "void checkUnit(UnitDecl* unit, ASTContext& ctxt,
//         DiagnosticEngine& engine)"

#pragma once

#include <cstdint>
#include <tuple>
#include <memory>
#include "Fox/AST/ASTFwdDecl.hpp"
#include "LocalScope.hpp"

namespace fox {
  class ASTContext;
  class DiagnosticEngine;
  class Sema {
    public:
      //----------------------------------------------------------------------//
      // Type aliases
      //----------------------------------------------------------------------//

      // A shortened syntax for a std::pair of Type
      using TypePair = std::pair<Type, Type>;

      // The type used to represent Integral type ranks
      using IntegralRankTy = std::uint8_t;

      //----------------------------------------------------------------------//
      // Public Sema Interface
      //----------------------------------------------------------------------//

      // Constructor
        // TODO: Once ASTContext contains the DiagnosticEngine, remove the 2nd
        //       argument.
      Sema(ASTContext& ctxt, DiagnosticEngine& diags);

      // Typechecks a ASTNode (expression, declaration or statement)
      // and it's children.
      //
      // Returns The node that should take this node's place. Note that the 
      // returned node will always be equal to the argument unless the ASTNode 
      // contains an Expr. Never returns nullptr.
      ASTNode checkNode(ASTNode node);

      // Typechecks an expression and it's children.
      // 
      // Returns the expression or another equivalent expression that should
      // replace it. Never nullptr.
      Expr* typecheckExpr(Expr* expr);

      // Return enum for typecheckExprOfType
      enum class CheckedExprResult { 
        // The expression was successfully typechecked, and matched 
        // was of the expected type, or equivalent.
        Ok, 

        // The expression was successfully typechecked, but was of
        // another type that did not match the one expected.
        NOk,

        // The expression couldn't be typechecked.
        Error
      };

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

      // Returns the DiagnosticEngine used by this Sema instance.
      DiagnosticEngine& getDiagnosticEngine();

      // Returns the ASTContext used by this Sema instance
      ASTContext& getASTContext();

    private:
      //----------------------------------------------------------------------//
      // Private implementation classes
      //----------------------------------------------------------------------//

      // RAII Objects
      class RAIIDeclCtxt;
      class RAIILocalScope;

      // Checkers
      class Checker;
      class DeclChecker;
      class StmtChecker;
      class ExprChecker;

      //----------------------------------------------------------------------//
      // Private methods
      //----------------------------------------------------------------------//

      //---------------------------------//
      // DeclContext management
      //---------------------------------//

      // Sets the current DeclContext and returns a RAII object that will,
      // upon destruction, restore the previous DeclContext.
      RAIIDeclCtxt setDeclCtxtRAII(DeclContext* dc);
      DeclContext* getDeclCtxt() const;
      bool hasDeclCtxt() const;

      //---------------------------------//
      // Scope Management
      //---------------------------------//

      // Creates a new scope and set localScope_ to that newly created instance.
      // Returns a RAII object that will, upon destruction, restore the LocalScope.
      RAIILocalScope enterNewLocalScopeRAII();
      LocalScope* getLocalScope() const;
      bool hasLocalScope() const;

      //----------------------------------------------------------------------//
      // Private members
      //----------------------------------------------------------------------//

      // The current active DeclContext.
      DeclContext* currentDC_ = nullptr;

      // The current active LocalScope
      LocalScope* localScope_ = nullptr;
      
      // the ASTContext and DiagnosticEngine
      ASTContext &ctxt_;
      DiagnosticEngine& diags_;
  };

  // Common base class for all Checker classes. This is used to DRY the code 
  // as every Checker class needs to access common classes such as the 
  // ASTContext and DiagnosticEngine
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
  class Sema::RAIIDeclCtxt {
      Sema& sema_;
      DeclContext* oldDC_ = nullptr;
    public:
      RAIIDeclCtxt(Sema& sema, DeclContext* dc) : sema_(sema) {
        oldDC_ = sema.getDeclCtxt();
        sema_.currentDC_ = dc;
      }

      ~RAIIDeclCtxt() {
        sema_.currentDC_ = oldDC_;
      }
  };

  // A Small RAII object that sets the current LocalScope [in a Sema instance]
  // to a new LocalScope instance, owned by this object.
  class Sema::RAIILocalScope {
    Sema& sema_;
    std::unique_ptr<LocalScope> scope_;
    public:
      // Create a new LocalScope whose parent is the current active
      // localScope (maybe null)
      RAIILocalScope(Sema& sema) : sema_(sema),
        scope_(std::make_unique<LocalScope>(sema_.localScope_)) {}

      // Needed for enterLocalScopeRAII factory function
      RAIILocalScope(RAIILocalScope&&) = default;

      ~RAIILocalScope() {
        // Restore the scope
        sema_.localScope_ = scope_->getParent();
      }
  };
}
