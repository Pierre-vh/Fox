////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Sema.cpp										
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
//	This file implements Sema methods related to Types
////------------------------------------------------------////

#include "Sema.hpp"
#include "Fox/AST/Type.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"

using namespace fox;

bool Sema::unifySubtype(Type* a, Type* b)
{
	assert(a && b && "Pointers cannot be null");

	a = a->ignoreLValue();
	b = b->ignoreLValue();

	// Returns true if a and b share the same subtype
	if (compareSubtypes(a, b))
		return true;

	// SemaTypes checks
	{
		// Now check if we don't have a substitution type somewhere
		auto* aSema = dyn_cast<SemaType>(a);
		auto* bSema = dyn_cast<SemaType>(b);

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
		
		// If both are semaTypes, return false.
		if (aSema && bSema)
			return false;
	}

	// Arrays
	{
		auto* aArr = dyn_cast<ArrayType>(a);
		auto* bArr = dyn_cast<ArrayType>(b);

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

	if (a->getKind() == b->getKind())
	{
		// Checking additional requirements for Primitive Types
		if (auto* aPrim = dyn_cast<PrimitiveType>(a))
		{
			auto* bPrim = cast<PrimitiveType>(b);

			// If a is integral, return true if b is too.
			if (aPrim->isIntegral())
				return bPrim->isIntegral();

			// Else only return true if the PrimitiveKinds match.
			return (aPrim->getPrimitiveKind() == bPrim->getPrimitiveKind());
		}

		// Checking additional requirements for Array Types
		if (auto* aArr = dyn_cast<ArrayType>(a))
		{
			auto* bArr = cast<ArrayType>(b);
			// Check elements types recursively for arrays.
			return compareSubtypes(aArr->getElementType(), bArr->getElementType());
		}
		
		// Return true if we don't have 2 SemaTypes
		return !isa<SemaType>(a);
	}

	return true;
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
