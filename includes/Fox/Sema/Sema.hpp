//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
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

      Sema(ASTContext& ctxt);

      // Performs semantic analysis on a single declaration and it's children
      void checkDecl(Decl* decl);

      // Returns the DiagnosticEngine used by this Sema instance.
      DiagnosticEngine& getDiagnosticEngine();

      // Returns the ASTContext used by this Sema instance
      ASTContext& getASTContext();

    private:
      // Children checkers and others
      class Checker;
      class DeclChecker;
      class StmtChecker;
      class ExprChecker;
      class ExprFinalizer;

      //---------------------------------//
      // Semantic analysis entry points for
      // checkers
      //---------------------------------//

      // Typechecks an expression and it's children.
      // 
      // Returns the expression or another equivalent expression that should
      // replace it. Never nullptr.
      Expr* typecheckExpr(Expr* expr);

      // Performs semantic analysis on an expression and it's children.
      //  Typechecks an expression that is expected to be of a certain type.
      //
      //  The expression is modified in place.
      //
      //  Returns true if the expression was of the type expected (or
      //  can be implicitely converted to that type), false otherwise.
      bool typecheckExprOfType(Expr*& expr, Type type);

      // Performs semantic analysis on an expression and it's children.
      //  Typechecks an expression which is used as a condition.
      //
      //  The expression is modified in place.
      //
      //  Returns true if the expr can be used as a condition, false otherwise.
      bool typecheckCondition(Expr*& expr);

      // Performs semantic analysis on a single statement and it's children.
      void checkStmt(Stmt* stmt);

      //---------------------------------//
      // Type related methods
      //
      // Type checking, comparison, ranking, etc.
      //---------------------------------//

      // The unification algorithm which uses the "default" comparator,
      // which basically allows numeric types to be considered equal.
      //
      // In short, the unification algorithm tries to make A = B
      // if possible, but due to the way Fox's semantics work this 
      // unification algorithm won't alter types unless they are CellTypes.
      //
      // Also, this function is commutative.
      bool unify(Type a, Type b);


      // Unification with a custom comparator function
      //
      // Unification will consider the types equal iff 
      // "comparator(a, b)" return true.
      //
      // In short, the unification algorithm tries to make A = B when possible.
      // Unification will never alter type, as they are immutable, however
      // it might remove
      bool unify(Type a, Type b, std::function<bool(Type, Type)> comparator);

      // Ignore LValue and removes layers of ArrayTypes
      // until this reaches a point where one of the types become basic.
      // Note that the result types may not be basic! The function will simply
      // stop unwrapping once one of them becomes basic.
      static TypePair unwrapAll(Type a, Type b);

      // Simplifies "type", replacing type variables with their 
      // substitution (using getSubstRecursive()). 
      //
      // This involves rebuilding the type.
      //
      // If a type variable doesn't have a substitution, returns nullptr.
      // If "type" doesn't contain a TypeVariable, returns "type".
      Type simplify(Type type);

      // Calls simplify(type). If it returns nullptr, returns "type"
      Type trySimplify(Type type);

      // Returns true if the type is considered "well formed".
      //
      // Currently, this just checks that the type isn't an ErrorType.
      //
      // Note that unbound types are considered well formed.
      static bool isWellFormed(Type type);

      // Calls "isWellFormed" on every type in types.
      static bool isWellFormed(ArrayRef<Type> types);

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
        // Workaround a clang bug
        LookupOptions() noexcept {}

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
      // Type variables management
      //---------------------------------//

      // Creates a new TypeVariable
      Type createNewTypeVariable();

      // Resets the TypeVariable counters & substitutions vector.
      void resetTypeVariables();

      std::uint16_t tyVarsCounter_ = 0;

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

      // RAII object for openNewScopeRAII and enterFuncScopeRAII
      class RAIILocalScope;

      // Creates a new scope and set localScope_ to that new instance.
      // Returns a RAII object that will, upon destruction, restore the LocalScope.
      //
      // Asserts that the current localScope is non nullptr.
      RAIILocalScope openNewScopeRAII();

      // Creates a new "root" scope for the FuncDecl fn.
      // Returns a RAII object that will, upon destruction, restore the LocalScope.
      //
      // Asserts that the current localScope is nullptr and that fn is not nullptr.
      RAIILocalScope enterFuncScopeRAII(FuncDecl* fn);

      // Return the currently active local scope, or nullptr if none is active.
      LocalScope* getLocalScope() const;

      // Returns true if this Sema instance posseses an active local scope in
      // which local declarations can be made visible
      bool hasLocalScope() const;

      //---------------------------------//
      // Other members
      //---------------------------------//

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
    LocalScope scope_;
    public:
      // Create a new LocalScope whose parent is the current active
      // localScope (maybe null)
      //
      // If FuncDecl is non null, it will be used as the parent of this
      // scope.
      RAIILocalScope(Sema& sema, FuncDecl* fn = nullptr) : sema_(sema) {
        if(fn) scope_.setParent(fn);
        else  scope_.setParent(sema_.localScope_);
        sema_.localScope_ = &scope_;
      }

      // Needed for openNewScopeRAII factory function
      RAIILocalScope(RAIILocalScope&&) = default;

      ~RAIILocalScope() {
        sema_.localScope_ = scope_.getParentIfLocalScope();
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
