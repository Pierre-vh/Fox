//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : SemaTypes.cpp										
// Author : Pierre van Houtryve								
//----------------------------------------------------------------------------//
//			SEE HEADER FILE FOR MORE INFORMATION			
//	This file implements Sema methods related to Types
//----------------------------------------------------------------------------//

#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/ConstraintVisitor.hpp"
#include "Fox/AST/Type.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"
#include <tuple>

using namespace fox;

namespace
{
	// Compares a and b, returning true if
	// a and b are strictly equal OR a and b are of the same family	
	// 
	// This function will also ignore LValues and unwrap array types.
	// It doesn't compare ConstrainedTypes and will return false if
	// a or b is one.
	bool compareSubtypes(Type a, Type b)
	{
		assert(a && b && "Pointers cannot be null");

		// Ignores LValues to perform the comparison.
		a = a->ignoreLValue();
		b = b->ignoreLValue();

		// Exact equality
		if (a == b)
			return true;

		// ConstrainedTypes
		if (a.is<ConstrainedType>() || b.is<ConstrainedType>())
			return false;

		// Check more in depth for some types of the same kind,
		// such as ArrayTypes.
		if (a->getKind() == b->getKind())
		{
			// Checking additional requirements for Primitive Types where
			// we allow 2 integrals to be considered "equal"
			if (isa<PrimitiveType>(a.getPtr()))
			{
				if (Sema::isIntegral(a) && Sema::isIntegral(b))
					return true;
				return false;
			}

			// Checking Array Types
			if (isa<ArrayType>(a.getPtr()))
			{
				TypeBase* elemA = a->unwrapIfArray();
				TypeBase* elemB = b->unwrapIfArray();

				assert(elemA && elemB && "Types are null");

				// Unwrap and check again
				return compareSubtypes(elemA, elemB);
			}
		}

		return false;
	}

	// Performs the pre-unifications tasks.
	// Returns true if unification can go on, false if it should
	// be aborted.
	bool performPreUnificationTasks(Type& a, Type& b)
	{
		assert(a && b && "Pointers cannot be nullptr");

		// ignore LValues, they don't matter when
		// unifying as they are never propagated.
		a = a->ignoreLValue();
		b = b->ignoreLValue();

		// If we have error types, unification is impossible.
		if (isa<ErrorType>(a.getPtr()) || isa<ErrorType>(b.getPtr()))
			return false;

		// Unwrap if both are arrays
		auto* arrA = a.getAs<ArrayType>();
		auto* arrB = b.getAs<ArrayType>();
		if (arrA && arrB)
		{
			Type unwrappedA = arrA->getElementType();
			Type unwrappedB = arrB->getElementType();
			return performPreUnificationTasks(unwrappedA, unwrappedB);
		}
		return true;
	}

	// Unwraps both values if they're both ArrayTypes.
	// Returns nullptr if no unwrapping was done.
	std::pair<TypeBase*,TypeBase*> 
	unwrapIfBothArrayTypes(TypeBase* a, TypeBase* b)
	{
		assert(a && b);
		a = a->unwrapIfArray();
		b = b->unwrapIfArray();
		if (a && b)
			return { a, b };
		return { nullptr, nullptr };
	}

	// Unwraps all layers of ArrayTypes until we reach a point where they are both
	// no longer arraytypes, or only one of them is.
	// Returns it's argument or the unwrapped types. 
	// Never returns nullptr.
	std::pair<TypeBase*, TypeBase*>
	recursivelyUnwrapArrayTypes(TypeBase* a, TypeBase* b)
	{
		assert(a && b);
		auto* tmpA = a;
		auto* tmpB = b;
		std::tie(tmpA, tmpB) = unwrapIfBothArrayTypes(a, b);
		// Unwrapping was performed, assign and continue.
		if (tmpA && tmpB)
			return recursivelyUnwrapArrayTypes(tmpA, tmpB);
		return { a, b };
	}

	// Tries to adjust the Constrained Type's substitution 
	// to be equal or better than the candidate.
	void tryAdjustConstrainedType(ConstrainedType* cons, TypeBase* candidate)
	{
		assert(cons && candidate);
		auto* sub = cons->getSubstitution();

		// u means unwrapped
		TypeBase* uSub = nullptr;
		TypeBase* uCand = nullptr;
		std::tie(uSub, uCand) = recursivelyUnwrapArrayTypes(sub, candidate);
		if (TypeBase* greatest = Sema::getHighestRankingType(uSub, uCand).getPtr())
		{
			// If the candidate is the "highest ranked type" of both types,
			// replace cons' sub with the candidate
			if (greatest == uCand)
				cons->setSubstitution(candidate);
		}
	}

	// Compare 2 constraints lists, returning true if they are strictly equal.
	// NOTE: Here strictly equal doesn't mean pointer equality. This function compares
	// the members of the Constraints when needed, so different instances with the same members
	// are still considered equal.
	bool compareConstraintLists(ConstraintList& a, ConstraintList& b)
	{
		class ConstraintComparer : public ConstraintVisitor<ConstraintComparer, bool, Constraint*>
		{
			public:
				bool visitArrayCS(Constraint* cs, Constraint* other)
				{
					// Only true if the other is a ArrayCS too
					return (cs == other) || other->is(Constraint::Kind::ArrayCS);
				}
		};

		// Return now if they have a different size.
		if (a.size() != b.size())
			return false;

		ConstraintComparer csc;
		for (std::size_t k = 0; k < a.size(); k++)
		{
			if (!csc.visit(a[k], b[k]))
				return false;
		}
		return true;
	}

	// Unification helper function: handles unification of a ConstrainedType with something that isn't
	// a ConstrainedType.
	bool unifyConstrainedWithNonConstrained(Sema& me, ConstrainedType* cs, Type type)
	{
		// Check if b can become A's substitution by checking if
		// B respects the constraints of A.
		if (me.checkConstraintOnType(cs->getConstraints(), type))
		{
			// B respects the constraints of A

			// A has no substitution, just set it
			if (!cs->hasSubstitution())
			{
				// Maybe I should use a Type& for the susbtitution,
				// to be done if the TypeBase*'s problematic!
				cs->setSubstitution(type.getPtr());
				return true;
			}
			// A already has a substitution, unify it with B
			else
			{
				Type csSub = cs->getSubstitution();
				if (me.unify(csSub, type))
				{
					// If we have 2 constrained type with equivalent substitution, see
					// if we can make any adjustement.
					tryAdjustConstrainedType(cs, type.getPtr());
					return true;
				}
				return false;
			}
		}
		// B can't become A's substitution, unification fails.
		return false;
	}

}	// anonymous namespace

bool Sema::unify(Type& aRef, Type& bRef)
{
	// Copy both references and work on theses because we'll only write to the
	// arguments in some specific situations.
	Type a = aRef;
	Type b = bRef;

	//std::cout << "unify(" << a->toDebugString() << ", " << b->toDebugString() << ")\n";
	assert(a && b && "Pointers cannot be null");

	// Pre-unification checks, if they fail, unification fails too.
	if (!performPreUnificationTasks(a, b))
		return false;

	// Return early if a and b share the same subtype (no unification needed)
	if (compareSubtypes(a, b) && !a.is<ConstrainedType>())
		return true;

	/* Unification logic */
	/* 1) A(ConstrainedType) = B
			-> B is a ConstrainedType
				-> A and B have the same constraints
					-> A and B both have a substitution
						-> return unify(a's sub, b's sub)
					-> Only A has one
						-> B's sub becomes the same as A's, return true
					-> Only B has one
						-> A's sub becomes the same as B's, return true
					-> Both don't have one
						-> Change A's TypeBase* to be the same as B's
				-> Else return false
			-> B isn't a ConstrainedType
				-> return unifyConstrainedWithNonConstrained(...)
		2) A(Not ConstrainedType) = B(ConstrainedType)
			-> return unifyConstrainedWithNonConstrained(...)
		3) A(Not ConstrainedType) = B(Not ConstrainedType)
			-> If A and B are arrays
				-> Unwrap them and retur unify(a's elemTy, b's elemTy)
			-> Else return false.
		*/		

	// ConstrainedType = (Something)
	if (auto* aCS = a.getAs<ConstrainedType>())
	{
		// A constrained type should ALWAYS have at least one Constraint.
		assert(aCS->numConstraints() > 0 && "Empty constraint set");

		// ConstrainedType = ConstrainedType
		if (auto* bCS = b.getAs<ConstrainedType>())
		{
			// TODO: Review this code, it seems right but too good to be true

			assert(bCS->numConstraints() > 0 && "Empty constraint set");

			// Check if A and B have the same constraints
			if (compareConstraintLists(aCS->getConstraints(), bCS->getConstraints()))
			{
				Type aSub = aCS->getSubstitution();
				Type bSub = bCS->getSubstitution();
				// Both have a substitution, unify them !
				if (aSub && bSub)
				{
					if (aSub != bSub)
					{
						Type highest = Sema::getHighestRankingType(aSub, bSub, true, true);
						assert(highest && "unhandled case");
						bCS->setSubstitution(highest.getPtr());
					}
				}
				// A has a substitution, but B doesn't.
				else if (aSub)
					bCS->setSubstitution(aSub.getPtr());
				// B has a Substitution, but A doesn't.
				// else if (bSub)
				//	aCS->setSubstitution(bSub.getPtr());
				// Both have no substitution.

				// Here subs are now equal, so make aRef equal to bRef and return.
				aRef = bRef;
				return true;
			}
			else
				return false;
		}
		// ConstrainedType = (Not ConstrainedType)
		else
			return unifyConstrainedWithNonConstrained(*this, aCS, b);
	}
	// (Not ConstrainedType) = ConstrainedType
	else if (auto* bCS = b.getAs<ConstrainedType>())
		return unifyConstrainedWithNonConstrained(*this, bCS, a);
	// ArrayType = (Something)
	else if(auto* aArr = a.getAs<ArrayType>())
	{
		// Only succeeds if B is an ArrayType
		auto* bArr = b.getAs<ArrayType>();
		if (!bArr) return false;

		// Unify the element types.
		Type aArr_elem = aArr->getElementType();
		Type bArr_elem = bArr->getElementType();
		assert(aArr_elem && bArr_elem && "Null array element type");
		return unify(aArr_elem, bArr_elem);
	}
	// Unhandled
	return false;
}

bool Sema::checkConstraintOnType(ConstraintList& cs, Type ty)
{
	// checkConstraintOnType helper class.
	// Checks if the type passed as argument respects the constraint
	// if so, returns the type or the unwrapped type.
	// e.g. 
		// visit(ArrayCS, ArrayType) -> returns ArrayType::getElementType
	// Returns nullptr on failure.

	// Note: this is currently very empty because I only have 1 constraint. More may come in the future,
	// but if I manage to do everything without needing any more constraints, I'll remove this and use 
	// a quicker version that doesn't call the visitor.
	class ConstraintCheck : public ConstraintVisitor<ConstraintCheck, TypeBase*, TypeBase*>
	{
		public:
			using inherited = ConstraintVisitor<ConstraintCheck, TypeBase*, TypeBase*>;
			Sema& sema;

			ConstraintCheck(Sema& sema) :
				sema(sema)
			{

			}

			TypeBase* visitArrayCS(Constraint*, TypeBase* ty)
			{
				if (auto arr = dyn_cast<ArrayType>(ty))
				{
					auto* elemTy = arr->getElementType();
					assert(elemTy
						&& "The type must have an element type");
					return elemTy;
				}
				return nullptr;
			}
	};

	// Function logic

	TypeBase* tmp = ty.getPtr();

	// Check every constraint in the list individually.
	ConstraintCheck check(*this);
	for (Constraint* elem : cs)
	{
		tmp = check.visit(elem, tmp);

		if (!tmp) break;
	}

	return tmp; // tmp != nullptr
}

bool Sema::isIntegral(Type type)
{
	if (auto* prim = type.getAs<PrimitiveType>())
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

Type Sema::getHighestRankingType(Type a, Type b, bool ignoreLValues, bool unwrapTypes)
{
	// Backup the original type before we do anything with them.
	Type ogA = a, ogB = b;

	assert(a && b && "Pointers cannot be null");

	if (ignoreLValues)
	{
		a = a->ignoreLValue();
		b = b->ignoreLValue();
	}

	if (unwrapTypes)
	{
		std::tie(a, b) = recursivelyUnwrapArrayTypes(a.getPtr(), b.getPtr());
		assert(a && b && "Types are null after unwrapping?");
		assert(!a.is<ArrayType>() && !b.is<ArrayType>() 
			&& "Arrays should have been unwrapped!");
	}

	if (a == b)
		return ogA;

	if (isIntegral(a) && isIntegral(b))
	{
		if (getIntegralRank(a) > getIntegralRank(b))
			return ogA;
		return ogB;
	}
	return nullptr;
}

Sema::IntegralRankTy Sema::getIntegralRank(Type type)
{
	using Pk = PrimitiveType::Kind;

	assert(type && isIntegral(type)
		&& "Can only use this on a valid pointer to an integral type");

	auto* prim = cast<PrimitiveType>(type.getPtr());

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

bool Sema::isStringType(TypeBase* type)
{
	if (auto* prim = dyn_cast<PrimitiveType>(type))
		return prim->isString();
	return false;
}