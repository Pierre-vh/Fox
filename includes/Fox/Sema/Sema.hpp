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

namespace fox
{
	class Sema
	{
		public:
			Sema(ASTContext& ctxt);

			// Typedefs
			using IntegralRankTy = std::uint8_t;

			// The unification algorithms for types of the same subtypes.
			//
			// SemaType with no subs. + Any type -> True, sets appropriate subst
			// Any Type & Any Type -> returns compareSubtype(a,b)
			// False in all other cases.
			static bool unifySubtype(Type* a, Type* b);

			// Compares the Subtypes of A and B. Returns true if a and b
			// share the same "subtype", false otherwise.
			static bool compareSubtypes(Type* a, Type* b);

			// Given 2 types of the same subtype, return the highest ranking type.
			// Returns nullptr if the 2 types don't share the same subtype.
			static Type* getHighestRankingType(Type* a, Type* b);

			// This method returns the integral rank that a given type has.
			// type must not be null and must point to a arithmetic type.
			static IntegralRankTy getIntegralRank(PrimitiveType* type);

		private:
			ASTContext &ctxt_;
	};
}