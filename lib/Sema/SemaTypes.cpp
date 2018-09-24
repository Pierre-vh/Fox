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

bool Sema::unifySubtype(Type* a, Type* b)
{
	assert(a && b && "Pointers cannot be null");

	if (a == b)
		return true;

	if (isa<ErrorType>(a) || isa<ErrorType>(b))
		return false;

	// Return early if a and b share the same subtype (no unification needed) and that they
	// aren't ErrorTypes, return early
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
			return false;
		}
	}

	// Array checks
	{
		auto* aArr = dyn_cast<ArrayType>(a->ignoreLValue());
		auto* bArr = dyn_cast<ArrayType>(b->ignoreLValue());

		// Check arrays recursively
		if (aArr && bArr)
			return unifySubtype(aArr->getElementType(), bArr->getElementType());
	}

	// All other cases are false for now.
	return false;
}

bool Sema::compareSubtypes(Type* a, Type* b)
{
	assert(a && b && "Pointers cannot be null");

	// Ignore LValues
	a = a->ignoreLValue();
	b = b->ignoreLValue();

	// Early return for exact equality
	if (a == b)
		return true;

	// Check more in depth
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

Type* Sema::getHighestRankingType(Type* a, Type* b)
{
	if (!(a && b))
		return nullptr;

	// Ignore LValues since they won't be "propagated" anyway
	a = a->ignoreLValue();
	b = b->ignoreLValue();

	// If they share the same subtype
	if (compareSubtypes(a, b))
	{
		// Same subtype means (a == b) or (a and b) are both
		// integrals
		if (auto* pA = dyn_cast<PrimitiveType>(a))
		{
			auto* pB = cast<PrimitiveType>(b);
			if (pA->isIntegral())
			{
				assert(pB->isIntegral());
				if (getIntegralRank(pA) > getIntegralRank(pB))
					return a;
				return b;
			}
		}
		assert((a == b) && "Unimplemented situation");
		return a;
	}
	return nullptr;
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
			fox_unreachable("Unknown arithmetic type");
	}
}

bool Sema::tryJoinSemaTypes(Type* a, Type* b)
{
	SemaType* aSema = dyn_cast_or_null<SemaType>(a);
	SemaType* bSema = dyn_cast_or_null<SemaType>(b);

	if (aSema && bSema)
	{
		if (aSema->hasSubstitution() && bSema->hasSubstitution())
			return false;

		if (aSema->hasSubstitution())
			bSema->setSubstitution(aSema);
		else  // else, bSema has a subst or doesn't, but we do the same thing in both cases
			aSema->setSubstitution(bSema);
		return true;
	}
	return false;
}