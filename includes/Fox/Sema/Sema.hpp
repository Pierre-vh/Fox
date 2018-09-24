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

			// The unification algorithms for types of the same subtypes.
			//
			// SemaType with no subs. + Any type -> True, sets appropriate subst
			// Any Type & Any Type -> returns compareSubtype(a,b)
			// False in all other cases.
			// Due to the way Fox's semantics work
			// This unification algorithm won't change the types unless
			// they are SemaTypes. Refer to getHighestRankingType to "mix" 
			// 2 types.
			static bool unifySubtype(Type* a, Type* b);

			// Compares the Subtypes of A and B. Returns true if a and b
			// share the same "subtype", false otherwise.
			static bool compareSubtypes(Type* a, Type* b);

			// Given 2 types of the same subtype
				// If they are integrals, return the highest ranking integral's type
				// else it returns its first argument
			// Returns nullptr otherwise.
			static Type* getHighestRankingType(Type* a, Type* b);

			// This method returns the integral rank that a given type has.
			// type must not be null and must point to a arithmetic type.
			static IntegralRankTy getIntegralRank(PrimitiveType* type);


			DiagnosticEngine& getDiagnosticEngine();
			ASTContext& getASTContext();
		private:
			ASTContext &ctxt_;
			DiagnosticEngine& diags_;
	};
}