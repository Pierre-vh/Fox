////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : SemaTypes.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
//	This file implements Sema methods related to Types
////------------------------------------------------------////

#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/Type.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"

using namespace fox;

namespace
{
	bool compareSubtypes(Type* a, Type* b)
	{
		assert(a && b && "Pointers cannot be null");

		// Early return for exact equality
		if (a == b)
			return true;

		// Check more in depth for same kind
		if (a->getKind() == b->getKind())
		{
			// Checking additional requirements for Primitive Types where
			// exact equality
			if (auto* aPrim = dyn_cast<PrimitiveType>(a))
			{
				auto* bPrim = cast<PrimitiveType>(b);

				// If a is integral, return true if b is too.
				if (aPrim->isIntegral())
					return bPrim->isIntegral();

				// We can return false otherwise, because as Primitive types are
				// all singletons, if they shared the same PrimitiveKind the
				// if(a==b) up there would have caught that.
				return false;
			}

			// Checking Array Types
			if (isa<ArrayType>(a))
			{
				Type* elemA = a->unwrapIfArray();
				Type* elemB = b->unwrapIfArray();

				assert(elemA && elemB && "Types are null");

				// Check elements types recursively for arrays.
				return compareSubtypes(elemA, elemB);
			}

			// Check sematypes, we might unwrap them to compare their substitution
			// if they both have one
			if (isa<SemaType>(a))
			{
				auto* aSubst = cast<SemaType>(a)->getSubstitution();
				auto* bSubst = cast<SemaType>(b)->getSubstitution();

				if (aSubst && bSubst)
					return compareSubtypes(aSubst, bSubst);
				return false;
			}

			// Lastly, return true unless we have 2 ErrorTypes
			return !isa<ErrorType>(a);
		}

		return false;
	}

	// If type is a SemaType with a substitution, returns it.
	// if that substitution is also a SemaType, calls 
	// this function recursively until we reach a SemaType
	// with no sub or something that isn't a sub.
	// If type doesn't have a sub or isn't a SemaType, leaves the type
	// untouched.
	// Returns true if the type changed, false otherwise.
	bool prepareSemaTypeForUnification(Type*& type)
	{
		if (auto* sema = dyn_cast<SemaType>(type))
		{
			if (Type* sub = sema->getSubstitution())
			{
				// SemaType with sub, recurse if needed.
				if (isa<SemaType>(type))
				{
					prepareSemaTypeForUnification(type);
					return true;
				}
				// Else just return the sub.
				type = sub;
				return true;
			}
			// SemaType with no sub, don't do anything
			// special
			return false;
		}
		// Not a SemaType, don't change anything
		return false;
	}

	// Performs the pre-unifications tasks
	// Returns false if unification will fail and we can
	// return immediatly.
	bool performPreUnificationTasks(Type*& a, Type*& b)
	{
		assert(a && b && "Pointers cannot be nullptr");

		// ignore LValues, they don't matter when
		// unifying as they are never propagated.
		a = a->ignoreLValue();
		b = b->ignoreLValue();

		// If we have error types, unification is impossible.
		if (isa<ErrorType>(a) || isa<ErrorType>(b))
			return false;

		// handle SemaType unwrapping
		{
			bool rA = prepareSemaTypeForUnification(a);
			bool rB = prepareSemaTypeForUnification(b);
			// If one of them changed, recurse.
			if (rA || rB)
				return performPreUnificationTasks(a, b);
		}

		// handle ArrayType unwrapping
		{
			auto* arrA = dyn_cast<ArrayType>(a);
			auto* arrB = dyn_cast<ArrayType>(b);

			// Both are arrays, unwrap & recurse
			if (arrA && arrB)
			{
				a = arrA->getElementType();
				b = arrB->getElementType();
				assert(a && b && "Array had a null element type");
				return performPreUnificationTasks(a, b);
			}
			// Only one of them is an array, unification fails
			else if ((!arrA) != (!arrB))
				return false;
			// None of them are arrays, keep going
		}

		// If we didn't return yet, mission success!
		return true;
	}

}	// anonymous namespace

bool Sema::unifySubtype(Type* a, Type* b)
{
	assert(a && b && "Pointers cannot be null");

	// Pre-unification checks, if they fail, return.
	if (!performPreUnificationTasks(a, b))
		return false;

	// Return early if a and b share the same subtype (no unification needed)
	if (compareSubtypes(a, b))
		return true;

	// SemaTypes checks
	{
		// Now check if we don't have a substitution type somewhere
		auto* aSema = dyn_cast<SemaType>(a->ignoreLValue());
		auto* bSema = dyn_cast<SemaType>(b->ignoreLValue());

		// if a or b is a SemaType
		if ((!aSema) != (!bSema))
		{
			// Set the substitution to the other type.
			if (aSema)
				aSema->setSubstitution(b);
			else
				bSema->setSubstitution(a);

			return true;
		}
		
		// If both are semaTypes
		if (aSema && bSema)
		{
			// if one of them has a subst and the other doesn't
			if (aSema->hasSubstitution() != bSema->hasSubstitution())
			{
				if (aSema->hasSubstitution())
					bSema->setSubstitution(aSema->getSubstitution());
				else 
					aSema->setSubstitution(bSema->getSubstitution());
				return true;
			}
			// Both have none
			else if (!aSema->hasSubstitution())
			{
				// In this case, we make A's sub B
				// thus A becomes SemaType(SemaType(nullptr)
				aSema->setSubstitution(b);
				return true;
			}
			return false;
		}

		// None of them are SemaTypes
	}

	// Arrays don't need special handling as
	// they're unwrapped in performPreUnificationTasks

	// All other cases are false for now.
	return false;
}

Type* Sema::getHighestRankingType(Type* a, Type* b)
{
	assert(a && b && "Pointers cannot be null");

	// Ignore LValues since they won't be "propagated" anyway
	a = a->ignoreLValue();
	b = b->ignoreLValue();

	// If they're different
	if (a != b)
	{
		// If they're different, unless we face 2 primitive
		// integral types, they don't share the same family.
		auto* pA = dyn_cast<PrimitiveType>(a);
		auto* pB = dyn_cast<PrimitiveType>(b);
		if(pA && pB)
		{
			if (pA->isIntegral() && pB->isIntegral())
			{
				if (getIntegralRank(pA) > getIntegralRank(pB))
					return a;
				return b;
			}
		}
		return nullptr;
	}

	// They're equal, return a
	return a;
}

Sema::IntegralRankTy Sema::getIntegralRank(PrimitiveType* type)
{
	using Ty = PrimitiveType::Kind;

	assert(type && type->isIntegral()
		&& "Can only use this on a valid pointer to an integral type");

	switch (type->getPrimitiveKind())
	{
		case Ty::BoolTy:
			return 0;
		case Ty::IntTy:
			return 1;
		case Ty::FloatTy:
			return 2;
		default:
			fox_unreachable("Unknown integral type");
	}
}