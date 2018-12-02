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

namespace fox {
  class ASTContext;
  class Sema {
    public:
      Sema(ASTContext& ctxt, DiagnosticEngine& diags);

      // Typedefs
      using TypePair = std::pair<Type, Type>;
      using IntegralRankTy = std::uint8_t;

      // Typechecks an expression. Returns the
      // expression, or another one that should
      // take it's place, or nullptr if critical
      // failure (such as ill formed ast)
      Expr* typecheckExpr(Expr* expr);

      // The unification algorithms for types of the same subtypes.
      // Tries to make A = B
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
      //    e.g. [int] & [int] is unwrapped to 
      //          int & int but [[int]] & [int] is unwrapped to [int] & int
      // if ignoreLValues is set to true, lvalues are ignored prior to 
      // comparison.
      static Type getHighestRankedTy(Type a, Type b,
        bool unwrap = true);

      // This method returns the integral rank that a given type has.
      // type must not be null and must point to a arithmetic type.
      static IntegralRankTy getIntegralRank(Type type);;

      // Walk the type, returns false if it contains a unbound
      // CellType.
      static bool isBound(Type ty);

      // Given a type, return the Basic type if it can find one, or nullptr.
      // e.g.
      //    LValue(Array(Array(int))) will return int
      static BasicType* findBasicType(Type type);

      // Removes the same number of ArrayType layers on 2 types
      static TypePair unwrapArrays(TypePair pair);

      // Removes all layers of LValue, CellType and ArrayType 
      // until this reaches a point where one (or both) of the
      // types become basic.
      // Note that both types may not be basic! The function will simply
      // stop unwrapping once one of them becomes basic.
      static TypePair unwrapAll(TypePair pair);

      DiagnosticEngine& getDiagnosticEngine();
      ASTContext& getASTContext();

    private:
      ASTContext &ctxt_;
      DiagnosticEngine& diags_;
  };
}
