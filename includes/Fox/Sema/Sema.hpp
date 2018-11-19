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
#include "Fox/AST/ASTNode.hpp"
#include "Fox/AST/ASTFwdDecl.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"

#define DEBUG_SEMA 0

#if DEBUG_SEMA
  #define SEMA_DBG(x) std::cout << x << "\n"
#else
  #define SEMA_DBG(x) while(0)
#endif

namespace fox {
  class ASTContext;
  class Sema {
    public:
      Sema(ASTContext& ctxt, DiagnosticEngine& diags);

      // Typedefs
      using IntegralRankTy = std::uint8_t;

      // Typechecks an expression. Returns the
      // expression, or another one that should
      // take it's place, or nullptr if critical
      // failure (such as ill formed ast)
      Expr* typecheckExpr(Expr* expr);

      // The unification algorithms for types of the same subtypes.
      // Tries to make A = B
      //
      // Behaviour: TBA
      //
      // Due to the way Fox's semantics work
      // This unification algorithm won't alter types unless
      // they are CellTypes.
      //
      // Also, this function is commutative.
      bool unify(Type a, Type b);

      // Returns true if a is a PrimitiveType of
      // type Int/Float/Bool
      static bool isIntegral(Type type);

      // Given 2 types
        // If they are integrals, return the highest ranking integral's type
        // If they are equal, return it's first argument
        // Returns nullptr otherwise.
      // if unwrapTypes is set to true, types are unwrapped together.
      //    e.g. [int] & [int] is unwrapped to int & int but [[int]] & [int] is unwrapped to [int] & int
      // if ignoreLValues is set to true, lvalues are ignored prior to comparison.
      static Type getHighestRankingType(Type a, Type b, bool ignoreLValues = false, bool unwrapTypes = false);

      // This method returns the integral rank that a given type has.
      // type must not be null and must point to a arithmetic type.
      static IntegralRankTy getIntegralRank(Type type);

      // Returns true if "type" is a string type.
      static bool isStringType(TypeBase* type);

      // Walk the type, returns false if it contains a unbound
      // CellType
      static bool isBound(TypeBase* ty);

      // If "type" is a CellType with a substitution, returns it
      static TypeBase* deref(TypeBase* type, bool recursive = true);

      DiagnosticEngine& getDiagnosticEngine();
      ASTContext& getASTContext();

    private:
      ASTContext &ctxt_;
      DiagnosticEngine& diags_;
  };
}