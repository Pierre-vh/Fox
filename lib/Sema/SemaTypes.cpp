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
#include "Fox/AST/ConstraintVisitor.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"
#include <tuple>

using namespace fox;

namespace
{
	bool compareSubtypes(Type a, Type b)
	{
		assert(a && b && "Pointers cannot be null");

		// Early return for exact equality
		if (a == b)
			return true;

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
		while (true)
		{
			std::tie(tmpA, tmpB) = unwrapIfBothArrayTypes(a, b);
			// no unwrapping was performed, return
			if (tmpA && tmpB)
			{
				a = tmpA;
				b = tmpB;
			}
			return { a, b };
		}
	}

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
	// Return true if both ConstraintLists are identical.
	bool compareConstraintLists(ConstraintList& a, ConstraintList& b)
	{
		if (a.size() != b.size())
			return false;

		for (std::size_t k(0); k < a.size(); k++)
		{
			// TODO: Compare them more in depth, this works currently
			// but won't in the future when I'll start using EqualityCS
			if (a[k] != b[k])
				return false;
		}
		return true;
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

	// Pre-unification checks, if they fail, return.
	if (!performPreUnificationTasks(a, b))
		return false;

	// Return early if a and b share the same subtype (no unification needed)
	if (compareSubtypes(a, b) && !a.is<ConstrainedType>())
		return true;

	// Unification's impl
	// This lambda returns 2 bool, the first one is true if
	// the case was handled (it is not needed to call doIt again)
	// false otherwise, the 2nd is the actual result of the unification.
	auto doIt = [&]() -> std::pair<bool, bool> {
		if (auto* aCS = a.getAs<ConstrainedType>())
		{
			assert(aCS->numConstraints() > 0 && "Empty constraint set");
			// Constrained Type with Constrained Type
			if (auto* bCS = b.getAs<ConstrainedType>())
			{
				assert(bCS->numConstraints() > 0 && "Empty constraint set");

				if (compareConstraintLists(aCS->getConstraints(), bCS->getConstraints()))
				{
					Type aSub = aCS->getSubstitution();
					Type bSub = bCS->getSubstitution();
					// Both have a substitution
					if (aSub && bSub)
					{
						// TODO: rework this by making them both use the same ConstrainedType instance
						// if(unify(aSub, bSub)) a = b = aCS; (or something like that)
						// else return false;
						return { true, unify(aSub, bSub) };
					}
					// A has a substitution, but B doesn't.
					else if (aSub)
						bCS->setSubstitution(aSub.getPtr());
					// B has a Substitution, but A doesn't.
					else if (bSub)
						aCS->setSubstitution(bSub.getPtr());
					// Both have no substitution.
					else
					{
						// Both have no substitution, but their ConstraintLists are identical:
						// make both Type& point to the same type by making b point to a;
						bRef = aRef;
					}
					return { true, true };
				}
				else
					return { true, false };
			}
			else // Constrained type with anything else, check if it's ok
			{
				if (checkConstraintOnType(aCS->getConstraints(), b))
				{
					// Maybe I should use a Type& for the susbtitution,
					// to be done if the TypeBase*'s problematic!
					if (!aCS->hasSubstitution())
						aCS->setSubstitution(b.getPtr());
					else
					{
						Type aCSSub = aCS->getSubstitution();
						if (unify(aCSSub, b))
						{
							// If we have 2 constrained type with equivalent substitution, see
							// if we can make any adjustement.
							tryAdjustConstrainedType(aCS, b.getPtr());
							return { true, true };
						}
						return { false, false };
					}
					// Else return true.
					return { true, true };
				}
			}
		}
		else if(auto* aArr = a.getAs<ArrayType>())
		{
			auto* bArr = b.getAs<ArrayType>();
			if (!bArr) return { false , false };

			Type aArr_elem = aArr->getElementType();
			Type bArr_elem = bArr->getElementType();
			assert(aArr_elem && bArr_elem && "Null array element type");
			return { true, unify(aArr_elem, bArr_elem) };
		}
		// Unhandled
		return { false, false };
	};

	// Try to unify a and b, if it fails, try with b and a.
	// Return result.second after that.
	auto result = doIt();
	if (!result.first)
	{
		// If it fails the first time (unhandled case), 
		// try again once more by swapping a and b.
		std::swap(a, b);
		result = doIt();
	}
	return result.second;
}

namespace
{
	// checkConstraintOnType helper class.
	// Checks if the type passed as argument respects the constraint
	// if so, returns the type or the unwrapped type.
	// e.g. 
		// visit(ArrayCS, ArrayType) -> returns ArrayType::getElementType
		// visit(EqualityCS, SomeType) -> returns the unified SomeType.
	// Returns nullptr on failure.
	class ConstraintCheck : public ConstraintVisitor<ConstraintCheck, TypeBase*, TypeBase*>
	{
		public:
			using inherited = ConstraintVisitor<ConstraintCheck, TypeBase*, TypeBase*>;
			Sema& sema;

			ConstraintCheck(Sema& sema) :
				sema(sema)
			{

			}

			/*
			TypeBase* visit(Constraint* visited, TypeBase* ty)
			{
				std::cout << "visit(" << visited->toDebugString() << "," << ty->toDebugString() << ") -> ";
				TypeBase* rtr = inherited::visit(visited, ty);
				std::cout << rtr->toDebugString() << "\n";
				return rtr;
			}
			*/

			TypeBase* visitArrayCS(ArrayCS*, TypeBase* ty)
			{
				if (auto arr = dyn_cast<ArrayType>(ty))
				{
					auto* elemTy = arr->getElementType();
					assert(elemTy && "Must have element type");
					return elemTy;
				}
				return nullptr;
			}

			TypeBase* visitEqualityCS(EqualityCS* cs, TypeBase* ty)
			{
				Type& eq = cs->getType();
				Type wrapped(ty);
				if (sema.unify(eq, wrapped))
					return wrapped.getPtr();
				return nullptr;
			}
	};
}

bool Sema::checkConstraintOnType(ConstraintList& cs, Type& ty)
{
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
	if (auto* prim = dyn_cast<PrimitiveType>(type.getPtr()))
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

Type Sema::getHighestRankingType(Type a, Type b)
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