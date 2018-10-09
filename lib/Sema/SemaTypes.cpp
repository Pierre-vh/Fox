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

	// If type is a SemaType with a substitution:
	// if that substitution is also a SemaType, calls 
	// this function recursively until we reach a SemaType
	// with no sub or something that isn't a sub.
	// If type doesn't have a sub or isn't a SemaType, returns the type
	// untouched.
	// Returns it's argument or the unwrapped type.
	Type prepareSemaTypeForUnification(Type type)
	{
		if (auto* sema = dyn_cast<SemaType>(type.getPtr()))
		{
			// Get the substitution
			if (TypeBase* sub = sema->getSubstitution())
			{
				// It has a sub, if it's SemaType, recurse to unwrap further.
				if(isa<SemaType>(sub))
					return prepareSemaTypeForUnification(sub);
			}
		}
		return type;
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

		return true;
	}

	// Tries to adjust the Sematype (uprank it if we can to match the candidate). 
	// Returns true on success, false otherwise.
	bool tryAdjustSemaType(SemaType* semaTy, Type candidate)
	{
		auto* sub = semaTy->getSubstitution();
		assert(sub && "Must have a sub");
		// if the sub is integral, we might be able to uprank
		if (Sema::isIntegral(sub) && Sema::isIntegral(candidate))
		{
			Type highest = Sema::getHighestRankingType(sub, candidate);
			assert(highest && "Can't find the highest rank between 2 integrals?");
			semaTy->setSubstitution(highest.getPtr());
			return true;
		}
		return false;
	}

	bool compareConstraintLists(ConstraintList& a, ConstraintList& b)
	{
		if (a.size() != b.size())
			return false;

		for (std::size_t k(0); k < a.size(); k++)
		{
			if (a[k] != b[k])
				return false;
		}
		return true;
	}
}	// anonymous namespace

bool Sema::unify(Type a, Type b)
{
	assert(a && b && "Pointers cannot be null");

	// Pre-unification checks, if they fail, return.
	if(!performPreUnificationTasks(a, b))
		return false;

	// Return early if a and b share the same subtype (no unification needed)
	if (compareSubtypes(a, b) && !a.is<ConstrainedType>())
		return true;

	// Unification's impl
	// This lambda returns 2 bool, the first one is true if
	// the case was handled (it is not needed to call doIt again)
	// false otherwise, the 2nd is the actual result of the unification.
	auto doIt = [this](Type& a, Type& b) -> std::pair<bool, bool> {
		if (auto* aCS = a.getAs<ConstrainedType>())
		{
			assert(aCS->numConstraints() > 0 && "Empty constraint set");
			// Constrained Type with Constrained Type
			if (auto* bCS = b.getAs<ConstrainedType>())
			{
				assert(bCS->numConstraints() > 0 && "Empty constraint set");

				if (compareConstraintLists(aCS->getConstraints(), bCS->getConstraints()))
				{
					TypeBase* aSub = aCS->getSubstitution();
					TypeBase* bSub = bCS->getSubstitution();
					if (aSub && bSub)
					{
						// TODO: If i use a Type& for unify this
						// will need to be reworked
						// 
						// if(unify(aSub, bSub)) a = b = aCS; (or something like that)
						// else return false;
						return { true, unify(aSub, bSub) };
					}
					else if (aSub)
						bCS->setSubstitution(aSub);
					else if (bSub)
						aCS->setSubstitution(bSub);
					else
					{
						// Both have no substitution, set a's sub to b.
						aCS->setSubstitution(bCS);
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
						// Already have a substitution, recurse 
						// TODO: This doesn't seem correct, tests needed.
						return { true, unify(aCS->getSubstitution(), b) };
					}
					// Else return true.
					return { true, true };
				}
			}
		}
		else if(auto* aArr = a.getAs<ArrayType>())
		{
			auto* bArr = b.getAs<ArrayType>();
			if (!bArr) return { true, false };

			return { true, unify(aArr->getElementType(), bArr->getElementType()) };
		}
		// Unhandled
		return { false, false };
	};

	// Try to unify a and b, if it fails, try with b and a.
	// Return result.second after that.
	auto result = doIt(a, b);
	if (!result.first)
		result = doIt(b, a);
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

Type Sema::deref(Type type)
{
	assert(type && "type cannot be null");
	if (auto* sema = dyn_cast<SemaType>(type.getPtr()))
		return sema->hasSubstitution() ? deref(sema->getSubstitution()) : type;
	return type;
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