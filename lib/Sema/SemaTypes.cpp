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
#include <tuple>

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
			// we allow 2 integrals to be considered "equal"
			if (isa<PrimitiveType>(a))
			{
				if (Sema::isIntegral(a) && Sema::isIntegral(b))
					return true;
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

	// If type is a SemaType with a substitution:
	// if that substitution is also a SemaType, calls 
	// this function recursively until we reach a SemaType
	// with no sub or something that isn't a sub.
	// If type doesn't have a sub or isn't a SemaType, returns the type
	// untouched.
	// Returns it's argument or the unwrapped type.
	Type* prepareSemaTypeForUnification(Type* type)
	{
		if (auto* sema = dyn_cast<SemaType>(type))
		{
			// Get the substitution
			if (Type* sub = sema->getSubstitution())
			{
				// It has a sub, if it's SemaType, recurse to unwrap further.
				if(isa<SemaType>(sub))
					return prepareSemaTypeForUnification(sub);
			}
		}
		return type;
	}

	// Performs the pre-unifications tasks
	std::pair<Type*,Type*> performPreUnificationTasks(Type* a, Type* b)
	{
		assert(a && b && "Pointers cannot be nullptr");

		// ignore LValues, they don't matter when
		// unifying as they are never propagated.
		a = a->ignoreLValue();
		b = b->ignoreLValue();

		// If we have error types, unification is impossible.
		if (isa<ErrorType>(a) || isa<ErrorType>(b))
			return { nullptr, nullptr };

		// handle SemaType unwrapping
		a = prepareSemaTypeForUnification(a);
		b = prepareSemaTypeForUnification(b);

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
				return { nullptr, nullptr };
			// None of them are arrays, keep going
		}

		// If we didn't return yet, mission success!
		return { a, b };
	}

	// Tries to adjust the Sematype. Returns true on success, false otherwise.
	bool tryAdjustSemaType(SemaType* semaTy, Type* candidate)
	{
		auto* sub = semaTy->getSubstitution();
		assert(sub && "Must have a sub");
		// if the sub is integral, we might be able to uprank
		if (Sema::isIntegral(sub) && Sema::isIntegral(candidate))
		{
			auto* highest = Sema::getHighestRankingType(sub, candidate);
			assert(highest && "Can't find the highest rank between 2 integrals?");
			semaTy->setSubstitution(highest);
			return true;
		}
		return false;
	}

}	// anonymous namespace

bool Sema::unifySubtype(Type* a, Type* b)
{
	assert(a && b && "Pointers cannot be null");

	// Pre-unification checks, if they fail, return.
	std::tie(a, b) = performPreUnificationTasks(a, b);
	if (!a)
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
			// aSema is a SemaType
			if (aSema)
			{
				if (aSema->hasSubstitution()) // Don't overwrite a sub, adjust it or give up
					return tryAdjustSemaType(aSema, b);
				aSema->setSubstitution(b);
			}
			// bSema is a SemaType
			else
			{
				if (bSema->hasSubstitution()) // Don't overwrite a sub, adjust it or give up
					return tryAdjustSemaType(bSema, a);
				bSema->setSubstitution(a);
			}

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

bool Sema::isIntegral(Type* a)
{
	if (auto* prim = dyn_cast<PrimitiveType>(a))
	{
		using Pk = PrimitiveType::Kind;
		switch (prim->getPrimitiveKind())
		{
			case Pk::BoolTy:
			case Pk::FloatTy:
			case Pk::IntTy:
				return true;
			default:
				return false;
		}
	}
	return false;
}

Type* Sema::deref(Type* type)
{
	assert(type && "type cannot be null");
	if (auto* sema = dyn_cast<SemaType>(type))
		return sema->hasSubstitution() ? deref(sema->getSubstitution()) : type;
	return type;
}

Type* Sema::getHighestRankingType(Type* a, Type* b)
{
	assert(a && b && "Pointers cannot be null");

	if (a == b)
		return a;

	if (isIntegral(a) && isIntegral(b))
	{
		if (getIntegralRank(a) > getIntegralRank(b))
			return a;
		return b;
	}
	return nullptr;
}

Sema::IntegralRankTy Sema::getIntegralRank(Type* type)
{
	using Pk = PrimitiveType::Kind;

	assert(type && isIntegral(type)
		&& "Can only use this on a valid pointer to an integral type");

	auto* prim = cast<PrimitiveType>(type);

	switch (prim->getPrimitiveKind())
	{
		case Pk::BoolTy:
			return 0;
		case Pk::IntTy:
			return 1;
		case Pk::FloatTy:
			return 2;
		default:
			fox_unreachable("Unknown integral type");
	}
}