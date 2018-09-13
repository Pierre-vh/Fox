////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Sema.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Contains the Sema class, which is used to perform 
// most of the semantic analysis of a Fox AST.
////------------------------------------------------------//// 

#include <cstdint>
#include "Fox/AST/ASTNode.hpp"

namespace fox
{
	class Type;
	class PrimitiveType;

	class Sema
	{
		public:
			// Typedefs
			using IntegralRankTy = std::uint8_t;

			// Sema::SemaResult encapsulates the result of a Semantic Analysis function, which
			// is a ASTNode (potentially nullptr) & a boolean result (for success/failure of checking.)
			class SemaResult
			{
				public:
					static SemaResult Success(ASTNode node = nullptr);
					static SemaResult Failure();

					bool wasSuccessful() const;

					explicit operator bool() const;

					const ASTNode getReplacement() const;
					ASTNode getReplacement();

					bool hasReplacement() const;
					
				private:
					SemaResult(bool success, ASTNode node);

					ASTNode node_;
					bool success_ : 1;
					// 7 Bits left in bitfield
			};

			// The unification algorithms for types of the same subtypes.
			//
			// SemaType with no subs. + Any type -> True
			// Any Type & Any Type -> returns compareSubtype(a,b)
			// False in all other cases.
			static bool unifySubtype(Type* a, Type* b);

			// Compares the Subtypes of A and B. Returns true if a and b
			// share the same "subtype", false otherwise.
			static bool compareSubtypes(Type* a, Type* b);

			// This method returns the integral rank that a given type has.
			// type must not be null and must point to a arithmetic type.
			static IntegralRankTy getIntegralRank(PrimitiveType* type);

		private:

	};
}