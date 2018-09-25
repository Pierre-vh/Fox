////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Sema.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Contains the Sema class, which is used to perform 
// most of the semantic analysis of a Fox AST.
////------------------------------------------------------//// 

#pragma once

#include <cstdint>
#include "Fox/AST/ASTNode.hpp"
#include "Fox/AST/ASTFwdDecl.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"

namespace fox
{
	class Sema
	{
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
			// Behaviour:
				// SemaType with no subs. + Any type -> True, sets appropriate subst
				// Any Type & Any Type -> returns compareSubtype(a,b)
				// False in all other cases.
			//
			// Due to the way Fox's semantics work
			// This unification algorithm won't alter types unless
			// they are SemaTypes.
			//
			// Also, most of the time, order doesn't matter, but it does
			// in a one case, for instance when we have 2 empty SemaTypes,
			// we Set A's subst to b and return true.
			static bool unifySubtype(Type* a, Type* b);

			// Returns true if a is a PrimitiveType of
			// type Int/Float/Bool
			static bool isIntegral(Type* a);

			// If type is a SemaType with a substitution,
			// returns the substitution. This function
			// is recursive to handle nested SemaTypes.
			static Type* deref(Type* type);

			// Given 2 types
				// If they are integrals, return the highest ranking integral's type
				// If they are equal, return it's first argument
				// Returns nullptr otherwise.
			static Type* getHighestRankingType(Type* a, Type* b);

			// This method returns the integral rank that a given type has.
			// type must not be null and must point to a arithmetic type.
			static IntegralRankTy getIntegralRank(Type* type);

			DiagnosticEngine& getDiagnosticEngine();
			ASTContext& getASTContext();
		private:
			ASTContext &ctxt_;
			DiagnosticEngine& diags_;
	};
}