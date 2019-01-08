//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
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
#include "Fox/AST/ASTVisitor.hpp"
#include <tuple>

using namespace fox;

bool Sema::unify(Type a, Type b) {
  auto comparator = [](Type a, Type b) {
    // Allow exact equality unless it's a CellType exact
    // equality.
    if(!a->is<CellType>()) {
      if (a == b) 
        return true;
    }
    return false;
  };
  return unify(a, b, comparator);
}

bool Sema::unify(Type a, Type b, std::function<bool(Type, Type)> comparator)  {
  assert(a && b && "Pointers cannot be null");

  // Unwrap 
  std::tie(a, b) = Sema::unwrapAll(a, b);

  // Check if well formed
  if(!isWellFormed({a, b})) return false;

  // Check for a early return using the comparator.
  if(comparator(a, b))
    return true;

  /* Unification logic */

  // CellType = (Something)
  if (auto* aCell = a->getAs<CellType>()) {
    // CellType = CellType
    if (auto* bCell = b->getAs<CellType>()) {
      // Both are CellTypes, check if they have a substitution
      Type aCellSub = aCell->getSubst();
      Type bCellSub = bCell->getSubst();
      // Both have a sub
      if (aCellSub && bCellSub) {

        bool unifyResult = unify(aCellSub, bCellSub, comparator);
        // If it's nested CellTypes, just return the unifyResult.
        if (aCellSub->is<CellType>() || bCellSub->is<CellType>())
          return unifyResult;

        // If theses aren't nested celltypes, return false on error.
        if (!unifyResult)
          return false;

        // Unification of the subs was successful.
        return true;
      }
      // A has a sub, B doesn't
      if (aCellSub) {
        bCell->setSubst(aCellSub);
        return true;
      }
      // B has a sub, A doesn't
      if (bCellSub) {
        aCell->setSubst(bCellSub);
        return true;
      }
      // None of them has a sub.
      auto* fresh = CellType::create(ctxt_);
      aCell->setSubst(fresh);
      bCell->setSubst(fresh);
      return true;
    }
    // CellType = (Not CellType)
    else {
      if (auto* aCellSub = aCell->getSubst().getPtr())
        return unify(aCellSub, b, comparator);
      aCell->setSubst(b);
      return true;
    }
  }
  // (Not CellType) = CellType
  else if (auto* bCell = b->getAs<CellType>()) {
    if (Type bCellSub = bCell->getSubst())
      return unify(a, bCellSub, comparator);
    bCell->setSubst(a);
    return true;
  }
  // ArrayType = (Something)
  else if(auto* aArr = a->getAs<ArrayType>()) {
    // Only succeeds if B is an ArrayType
    auto* bArr = b->getAs<ArrayType>();
    if (!bArr) return false;

    // Unify the element types.
    Type aArr_elem = aArr->getElementType();
    Type bArr_elem = bArr->getElementType();
    assert(aArr_elem && bArr_elem 
      && "Array element type cannot be null");
    return unify(aArr_elem, bArr_elem);
  }
  // Unhandled
  return false;
}

static Sema::TypePair unwrapArrays(Type a, Type b) {
  assert(a && b && "args cannot be null");
  Type uwA = a->unwrapIfArray();
  Type uwB = b->unwrapIfArray();
  // Unwrapping was performed, recurse.
  if (uwA && uwB) return unwrapArrays(uwA, uwB);
  // No unwrapping done, return.
  return {a, b};
}

Sema::TypePair Sema::unwrapAll(Type a, Type b) {
  assert(a && b && "args cannot be null");
  // Ignore LValues & deref both
	// Note: getAsBoundRValue is not desired here
	// because we want to support unbound types.
  auto uwA = a->getRValue()->deref();
  auto uwB = b->getRValue()->deref();
  // Unwrap arrays
  std::tie(uwA, uwB) = unwrapArrays(uwA, uwB);
  // If both changed, recurse, else, return.
  if ((uwA != a) && (uwB != b))
    return unwrapAll(uwA, uwB);
  return {uwA, uwB};
}

bool Sema::isWellFormed(Type type) {
  return !type->is<ErrorType>();
}

bool Sema::isWellFormed(ArrayRef<Type> types) {
  for(auto type: types) {
    if(!isWellFormed(type))
      return false;
  }
  return true;
}
