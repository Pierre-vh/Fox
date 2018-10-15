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
#include "Fox/AST/Constraints.hpp"	// ConstraintList
#include "Fox/Common/DiagnosticEngine.hpp"

namespace fox
{
	class ASTContext;
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
				// Any Type & Any Type -> returns true if they are of the same subtypes.
				// SemaTypes with no subs: Creates a new SemaType and sets boths subs to this new SemaType.
				// False in all other cases.
			//
			// Due to the way Fox's semantics work
			// This unification algorithm won't alter types unless
			// they are SemaTypes.
			//
			// Also, this function is NOT commutative,
			// unify(a,b) might succeed when unify(b,a) fails.
			bool unify(Type& aRef, Type& bRef);

			// Checks if the type ty "respects" every constraint in cs.
			// Return true on success, false otherwise.
			bool checkConstraintOnType(ConstraintList& cs, Type ty);

			// Returns true if a is a PrimitiveType of
			// type Int/Float/Bool
			static bool isIntegral(Type type);

			// Given 2 types
				// If they are integrals, return the highest ranking integral's type
				// If they are equal, return it's first argument
				// Returns nullptr otherwise.
			// if unwrapTypes is set to true, types are unwrapped together.
			//		e.g. [int] & [int] is unwrapped to int & int but [[int]] & [int] is unwrapped to [int] & int
			// if ignoreLValues is set to true, lvalues are ignored prior to comparison.
			static Type getHighestRankingType(Type a, Type b, bool ignoreLValues = false, bool unwrapTypes = false);

			// This method returns the integral rank that a given type has.
			// type must not be null and must point to a arithmetic type.
			static IntegralRankTy getIntegralRank(Type type);

			// Returns false if type is a ConstrainedType with no substitution, false
			// otherwise.
			static bool isMaterializable(Type t);

			DiagnosticEngine& getDiagnosticEngine();
			ASTContext& getASTContext();

		private:
			ASTContext &ctxt_;
			DiagnosticEngine& diags_;
	};
}