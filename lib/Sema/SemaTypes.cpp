//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : SemaTypes.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file implements Sema methods related to Types
//----------------------------------------------------------------------------//

#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/Type.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/AST/ASTWalker.hpp"
#include <tuple>

using namespace fox;

namespace {
  // Compares a and b, returning true if
  // a and b are strictly equal OR a and b are of the same family  
  // 
  // This function will also ignore LValues and unwrap array types.
  // It doesn't compare ConstrainedTypes and will return false if
  // a or b is one.
  bool compareSubtypes(Type a, Type b) {
    assert(a && b && "Pointers cannot be null");

    // Ignores LValues to perform the comparison.
    a = a->ignoreLValue();
    b = b->ignoreLValue();

    // Exact equality
    if (a == b)
      return true;

    // Check more in depth for some types of the same kind,
    // such as ArrayTypes.
    if (a->getKind() == b->getKind()) {
      // Checking additional requirements for Primitive Types where
      // we allow 2 integrals to be considered "equal"
      if (isa<PrimitiveType>(a.getPtr())) {
        if (Sema::isIntegral(a) && Sema::isIntegral(b))
          return true;
        return false;
      }

      // Checking Array Types
      if (isa<ArrayType>(a.getPtr())) {
        TypeBase* elemA = a->unwrapIfArray();
        TypeBase* elemB = b->unwrapIfArray();

        assert(elemA && elemB && "Types are null");

        // Unwrap and check again
        return compareSubtypes(elemA, elemB);
      }

      // Checking CellTypes (deref)
    }

    return false;
  }

  // Performs the pre-unifications tasks.
  // Returns true if unification can go on, false if it should
  // be aborted.
  bool performPreUnificationTasks(Type& a, Type& b) {
    assert(a && b && "Pointers cannot be nullptr");

    // Unwrap all
    std::tie(a, b) = Sema::unwrapAll({ a.getPtr(), b.getPtr() });

    // If we have error types, unification is impossible.
    if (isa<ErrorType>(a.getPtr()) || isa<ErrorType>(b.getPtr()))
      return false;
    return true;
  }

  // Tries to adjust the CellType's type 
  // to be equal or better than the candidate.
  void tryAdjustCellType(CellType* cell, TypeBase* candidate) {
    assert(cell && candidate);
    auto* sub = cell->getSubstitution();

    // u means unwrapped
    TypeBase* uSub = nullptr;
    TypeBase* uCand = nullptr;
    std::tie(uSub, uCand) = Sema::unwrapArrays({ sub, candidate });
    if (TypeBase* greatest = Sema::getHighestRankingType(uSub, uCand).getPtr()) {
      // If the candidate is the "highest ranked type" of both types,
      // replace cons' sub with the candidate
      if (greatest == uCand)
        cell->setSubstitution(candidate);
    }
  }
}  // anonymous namespace

bool Sema::unify(Type a, Type b) {
  assert(a && b && "Pointers cannot be null");

  // Pre-unification checks, if they fail, unification fails too.
  if (!performPreUnificationTasks(a, b))
    return false;

  // Return early if a and b share the same subtype (no unification needed)
  if (compareSubtypes(a, b) && !a.is<CellType>())
    return true;

  /* Unification logic */

  // CellType = (Something)
  if (auto* aCell = a.getAs<CellType>()) {
    // CellType = CellType
    if (auto* bCell = b.getAs<CellType>()) {
      // Both are CellTypes, check if they have a substitution
      auto* aCellSub = aCell->getSubstitution();
      auto* bCellSub = bCell->getSubstitution();
      // Both have a sub
      if (aCellSub && bCellSub) {

        bool unifyResult = unify(aCellSub, bCellSub);
        // If it's nested CellTypes, just return the unifyResult.
        if (isa<CellType>(aCellSub) || isa<CellType>(bCellSub))
          return unifyResult;

        // If theses aren't nested celltypes, return false on error.
        if (!unifyResult)
          return false;

        // Unification of the subs was successful. Check if they're different.
        if (aCellSub != bCellSub) {
          // If they're different, adjust both substitution to the highest
          // ranked type.
          TypeBase* highest = getHighestRankingType(aCellSub, bCellSub).getPtr();
          assert(highest); // Should have one since unification was successful
          aCell->setSubstitution(highest);
          bCell->setSubstitution(highest);
        }
        return true;
      }
      // A has a sub, B doesn't
      if (aCellSub) {
        bCell->setSubstitution(aCellSub);
        return true;
      }
      // B has a sub, A doesn't
      if (bCellSub) {
        aCell->setSubstitution(bCellSub);
        return true;
      }
      // None of them has a sub.
      auto* fresh = CellType::create(ctxt_);
      aCell->setSubstitution(fresh);
      bCell->setSubstitution(fresh);
      return true;
    }
    // CellType = (Not CellType)
    else {
      if (auto* aCellSub = aCell->getSubstitution())
        return unify(aCellSub, b);
      aCell->setSubstitution(b.getPtr());
      return true;
    }
  }
  // (Not CellType) = CellType
  else if (auto* bCell = b.getAs<CellType>()) {
    if (auto* bCellSub = bCell->getSubstitution())
      return unify(a, bCellSub);
    bCell->setSubstitution(a.getPtr());
    return true;
  }
  // ArrayType = (Something)
  else if(auto* aArr = a.getAs<ArrayType>()) {
    // Only succeeds if B is an ArrayType
    auto* bArr = b.getAs<ArrayType>();
    if (!bArr) return false;

    // Unify the element types.
    Type aArr_elem = aArr->getElementType();
    Type bArr_elem = bArr->getElementType();
    std::cout << "\t\tB is too. Recursing.\n";
    assert(aArr_elem && bArr_elem && "Null array element type");
    return unify(aArr_elem, bArr_elem);
  }
  // Unhandled
  return false;
}

bool Sema::isIntegral(Type type) {
  if (auto* prim = type.getAs<PrimitiveType>()) {
    using Pk = PrimitiveType::Kind;
    switch (prim->getPrimitiveKind()) {
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

Type Sema::getHighestRankingType(Type a, Type b, bool ignoreLValues, bool unwrapTypes) {
  // Backup the original type before we do anything with them.
  Type ogA = a, ogB = b;

  assert(a && b && "Pointers cannot be null");

  if (ignoreLValues) {
    a = a->ignoreLValue();
    b = b->ignoreLValue();
  }

  if (unwrapTypes) {
    std::tie(a, b) = Sema::unwrapArrays({ a.getPtr(), b.getPtr() });
    assert(a && b && "Types are null after unwrapping?");
    assert(!a.is<ArrayType>() && !b.is<ArrayType>() 
      && "Arrays should have been unwrapped!");
  }

  if (a == b)
    return ogA;

  if (isIntegral(a) && isIntegral(b)) {
    if (getIntegralRank(a) > getIntegralRank(b))
      return ogA;
    return ogB;
  }
  return nullptr;
}

Sema::IntegralRankTy Sema::getIntegralRank(Type type) {
  using Pk = PrimitiveType::Kind;

  assert(type && isIntegral(type)
    && "Can only use this on a valid pointer to an integral type");

  auto* prim = cast<PrimitiveType>(type.getPtr());

  switch (prim->getPrimitiveKind()) {
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

bool Sema::isStringType(TypeBase* type) {
  if (auto* prim = dyn_cast<PrimitiveType>(type))
    return prim->isString();
  return false;
}

bool Sema::isBound(TypeBase* ty) {
  class Impl : public ASTWalker {
    public:
      virtual bool handleTypePre(TypeBase* ty) override {
        if (auto* cell = dyn_cast<CellType>(ty))
          return cell->hasSubstitution();
        return true;
      }
  };
  return Impl().walk(ty);
}

TypeBase* Sema::deref(TypeBase* type) {
  if (auto* cell = dyn_cast<CellType>(type)) {
    TypeBase* sub = cell->getSubstitution();
    return sub ? deref(sub) : type;
  }
  return type;
}

Sema::TypeBasePair Sema::unwrapArrays(TypeBasePair pair) {
  assert(pair.first && pair.second && 
    "args cannot be null");
  auto* a = pair.first->unwrapIfArray();
  auto* b = pair.second->unwrapIfArray();
  // Unwrapping was performed, assign and continue.
  if (a && b)
    return unwrapArrays({ a, b });
  // No unwrapping done, return.
  return pair;
}

Sema::TypeBasePair Sema::unwrapAll(TypeBasePair pair) {
  auto tmp = pair;
  // Ignore LValues
  tmp.first = pair.first->ignoreLValue();
  tmp.second = pair.second->ignoreLValue();
  // Deref both
  tmp.first = Sema::deref(tmp.first);
  tmp.second = Sema::deref(tmp.second);
  // Unwrap arrays
  tmp = unwrapArrays(tmp);
  // If both changed, recurse.
  if ((tmp.first != pair.first) 
      && (tmp.second != pair.second))
    return unwrapAll(tmp);
  return tmp;
}