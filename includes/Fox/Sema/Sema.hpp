//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Sema.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Contains the public interface of the Fox Semantic Analysis class.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/AST/ASTFwdDecl.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"
#include <cstdint>
#include <tuple>
#include <functional>
#include <memory>

namespace fox {
  // Forward Declarations
  class ASTContext;
  class DiagnosticEngine;
  class SourceLoc;
  class Identifier;
  using NamedDeclVec = SmallVector<NamedDecl*, 4>;

  /// This is the class that handles Semantic Analysis of the Fox AST.
  class Sema {
    public:
      /// \param ctxt the ASTContext instance to use. ctxt.diagEngine
      /// will be the Diagnostic Engine used to emit diagnostics.
      Sema(ASTContext& ctxt);

      /// Performs Semantic Analysis on a UnitDecl
      void checkUnitDecl(UnitDecl* decl);

      /// The ASTContext used to allocate nodes, etc.
      ASTContext& ctxt;

      /// The DiagnosticEngine used to emit diagnostics.
      DiagnosticEngine& diagEngine;

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

      // Typechecks an expression and its children.
      // 
      // Returns the expression or another equivalent expression that should
      // replace it. Never nullptr.
      Expr* typecheckExpr(Expr* expr);

      // Performs semantic analysis on an expression and its children.
      //  Typechecks an expression that is expected to be of a certain type.
      //
      //  The expression is modified in place.
      //
      //  Returns true if the expression was of the type expected (or
      //  can be implicitely converted to that type), false otherwise.
      bool typecheckExprOfType(Expr*& expr, Type type);

      // Performs semantic analysis on an expression and its children.
      //  Typechecks an expression which is used as a condition.
      //
      //  The expression is modified in place.
      //
      //  Returns true if the expr can be used as a condition, false otherwise.
      bool typecheckCondition(Expr*& expr);

      // Performs semantic analysis on a single statement and its children.
      void checkStmt(Stmt* stmt);

      // Performs semantic analysis on a declaration and its children.
      void checkDecl(Decl* decl);

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

      // Simplifies "type", replacing type variables with their 
      // substitution (using getSubstRecursive()). 
      //
      // This involves rebuilding the type.
      //
      // If a type variable doesn't have a substitution, returns nullptr.
      // If "type" doesn't contain a TypeVariable, returns "type".
      Type simplify(Type type);

      // Calls simplify(type). If it returns nullptr, returns its parameter.
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

      // Class that encapsulates the result of a Lookup request.
      class LookupResult;

      // Class that represents options passed to a lookup request.
      struct LookupOptions {
        // The "noexcept" is to workaround a Clang bug
        LookupOptions() noexcept {}

        // If this is set to true, Lookup will only happen
        // in local DeclContexts.
        bool onlyLookInLocalDeclContexts = false;

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
      // Other members
      //---------------------------------//

      // The current active DeclContext.
      DeclContext* currentDC_ = nullptr;
  };

  // Common base class for all Checker classes. This is used to DRY the code 
  // as every Checker class needs to access common classes such as the 
  // ASTContext and DiagnosticEngine
  class Sema::Checker {
    public:
      Sema& sema;
      DiagnosticEngine& diagEngine;
      ASTContext& ctxt;
    protected:
      Checker(Sema& sema) : sema(sema),
        diagEngine(sema.diagEngine), ctxt(sema.ctxt) {}

  };

  // A Small RAII object that sets the currently active DeclContext
  // for a Sema instance. Upon destruction, it will restore the 
  // Sema's currently active DeclContext to what it was before.
  class Sema::RAIIDeclCtxt {
    DeclContext* oldDC_ = nullptr;
    public:
      Sema& sema;

      RAIIDeclCtxt(Sema& sema, DeclContext* dc) : sema(sema) {
        oldDC_ = sema.getDeclCtxt();
        sema.currentDC_ = dc;
      }

      ~RAIIDeclCtxt() {
        sema.currentDC_ = oldDC_;
      }
  };

  // A small class which is an abstraction around a vector of
  // NamedDecl*
  class Sema::LookupResult {
    public:
      LookupResult() = default;

      // Add a result in this LookupResult
      void addResult(NamedDecl* decl);

      NamedDeclVec& getDecls();
      const NamedDeclVec& getDecls() const;

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
