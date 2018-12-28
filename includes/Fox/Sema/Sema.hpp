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

#pragma once

#include "LocalScope.hpp"
#include "Fox/AST/ASTFwdDecl.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"
#include <cstdint>
#include <tuple>
#include <memory>

namespace fox {
  // Forward Declarations
  class ASTContext;
  class DiagnosticEngine;
  class SourceLoc;
  using NamedDeclVec = SmallVector<NamedDecl*, 4>;

  // This is the class that handles semantic analysis of the Fox AST.
  class Sema {
    public:
      // A shortened syntax for a std::pair of Type
      using TypePair = std::pair<Type, Type>;

      // The type used to represent Integral type ranks
      using IntegralRankTy = std::uint8_t;

      Sema(ASTContext& ctxt);

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

        // The expression was successfully typechecked, but was of
        // a type of higher rank than the one expected
        Downcast,

        // The expression couldn't be typechecked.
        Error
      };

      // Performs semantic analysis on an expression and it's children.
      //  Typechecks an expression whose type is already known.
      //
      //  Returns a pair. The first element of the pair is a success indicator
      //  (seem CheckedExprResult). The second Expr* pointer
      //  is the expression or another equivalent expr that should replace it.
      std::pair<CheckedExprResult, Expr*> 
      typecheckExprOfType(Expr* expr, Type type);

      // Performs semantic analysis on a single statement and it's children.
      void checkStmt(Stmt* stmt);

      // Performs semantic analysis on a single declaration and it's children
      void checkDecl(Decl* decl);

      // Returns the DiagnosticEngine used by this Sema instance.
      DiagnosticEngine& getDiagnosticEngine();

      // Returns the ASTContext used by this Sema instance
      ASTContext& getASTContext();

    private:
      // Checkers
      class Checker;
      class DeclChecker;
      class StmtChecker;
      class ExprChecker;

      //---------------------------------//
      // Type related methods
      //
      // Type checking, comparison, ranking, etc.
      //---------------------------------//

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

      // Removes all layers of LValue, CellType and ArrayType 
      // until this reaches a point where one (or both) of the
      // types become basic.
      // Note that the result types may not be basic! The function will simply
      // stop unwrapping once one of them becomes basic.
      static TypePair unwrapAll(Type a, Type b);

      //---------------------------------//
      // Name binding 
      //---------------------------------//

      // Makes a local decl visible in the current scope. 
      //
      // If a decl with the same identifier already existed in this scope, 
      // it is overwritten. (see the second value of the result pair
      // to know if a Decl was overwritten or not)
      //
      // /!\ Asserts that decl->isLocal() returns true!
      //
      // Returns a pair of booleans:
      //    {false, false} if no insertion occured because there is no active
      //      local scope.
      //    {true, true} if the insertion occured without overwriting any
      //      previous declaration
      //    {true, false} if the insertion occured and replaced a previous
      //      decl.
      std::pair<bool, bool> addLocalDeclToScope(NamedDecl* decl);

      // Class that encapsulates the result of a Lookup request.
      class LookupResult;

      // Class that represents options passed to a lookup request.
      struct LookupOptions {
        // If this is set to false, the Lookup will stop after
        // looking in the current LocalScope (if there is one).
        bool canLookInDeclContext = true;

        // If this is set to true, the SourceLoc will be ignored
        // when performing lookup.
        bool canIgnoreLoc = false;

        // This lambda, if non-null, will be called each time
        // the lookup finds a valid lookup result.
        //
        // If "shouldIgnore(result)"
        // returns true, the result will be ignored and not added
        // to the LookupResult.
        std::function<bool(NamedDecl*)> shouldIgnore;
      };

      // Performs a unqualified lookup in the current context and scope.
      //    -> If a matching decl is found in the local scope, the searchs stops
      //    -> If the search reaches the DeclContext, every result is returned
      // if lookInDeclCtxt is set to false, we'll only look for
      // decls inside the current LocalScope.
      void doUnqualifiedLookup(LookupResult& results, Identifier id, 
        SourceLoc loc, const LookupOptions& options = LookupOptions());

      //---------------------------------//
      // DeclContext management
      //---------------------------------//

      // RAII object for enterDeclCtxtRAII
      class RAIIDeclCtxt;

      // Sets the current DeclContext and returns a RAII object that will,
      // upon destruction, restore the previous DeclContext.
      RAIIDeclCtxt enterDeclCtxtRAII(DeclContext* dc);

      // Returns the currently active DeclContext, or nullptr if there's
      // none.
      DeclContext* getDeclCtxt() const;

      // Returns true if there's a currently active DeclContext.
      bool hasDeclCtxt() const;

      //---------------------------------//
      // Scope Management
      //---------------------------------//

      // RAII object for enterLocalScopeRAII
      class RAIILocalScope;

      // Creates a new scope and set localScope_ to that newly created instance.
      // Returns a RAII object that will, upon destruction, restore the LocalScope.
      RAIILocalScope enterLocalScopeRAII();

      // Return the currently active local scope, or nullptr if none is active.
      LocalScope* getLocalScope() const;

      // Returns true if this Sema instance posseses an active local scope in
      // which local declarations can be made visible
      bool hasLocalScope() const;

      // The current active DeclContext.
      DeclContext* currentDC_ = nullptr;

      // The current active LocalScope
      LocalScope* localScope_ = nullptr;
      
      // the ASTContext and DiagnosticEngine
      ASTContext &ctxt_;
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
        scope_(std::make_unique<LocalScope>(sema_.localScope_)) {
        sema_.localScope_ = scope_.get();
      }

      // Needed for enterLocalScopeRAII factory function
      RAIILocalScope(RAIILocalScope&&) = default;

      ~RAIILocalScope() {
        // Restore the scope
        sema_.localScope_ = scope_->getParent();
      }
  };

  // A small class which is an abstraction around a vector of
  // NamedDecl*
  class Sema::LookupResult {
    public:
      LookupResult() = default;

      // Add a result in this LookupResult
      void addResult(NamedDecl* decl);

      NamedDeclVec& getResults();
      const NamedDeclVec& getResults() const;

      std::size_t size() const;

      // If there's only one result, return it. Else, returns nullptr.
      NamedDecl* getIfSingleResult() const;

      // Return true if this result is empty (size() == 0)
      bool isEmpty() const;

      // Return true if this result is ambiguous (size() > 1)
      bool isAmbiguous() const;

      NamedDeclVec::iterator begin();
      NamedDeclVec::const_iterator begin() const;

      NamedDeclVec::iterator end();
      NamedDeclVec::const_iterator end() const;
    private:
      NamedDeclVec results_;
  };
}
