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
    // Allow exact equality unless they're both
    // typevariables.
    if(a == b) {
      // False if it's a TypeVariableType, as they're handled
      // in the unification algorithm specifically.
      return !a->is<TypeVariableType>();
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

  // TypeVariable = (Something)
  if (auto* aTV = a->getAs<TypeVariableType>()) {
    Type aTVSubst = getSubstitution(aTV);
    // TypeVariable = TypeVariable
    //  Both are TypeVariable, check if they have substitutions
    if (auto* bTV = b->getAs<TypeVariableType>()) {
      Type bTVSubst = getSubstitution(bTV);
      // Both have a sub
      if (aTVSubst && bTVSubst)
        return unify(aTVSubst, bTVSubst, comparator);
      // A has a sub, B doesn't
      if (aTVSubst) {
        // Set B's substitution to A.
        setSubstitution(bTV, aTV, false);
        return true;
      }
      // B has a sub, A doesn't
      if (bTVSubst) {
        // Set A's substitution to B
        setSubstitution(aTV, bTV, false);
        return true;
      }
      // None of them have a substitution
      // FIXME: This isn't efficient.
      Type fresh = createNewTypeVariable();
      setSubstitution(aTV, fresh, false);
      setSubstitution(bTV, fresh, false);
      return true;
    }
    // TypeVariable = (Not TypeVariable)
    else {
      // If aTV has a subst, unify the subst with b.
    // else, use b as the subst for aTV
      if (aTVSubst)
        return unify(aTVSubst, b, comparator);
      setSubstitution(aTV, b, false);
      return true;
    }
  }
  // (Not TypeVariable) = TypeVariable
  else if (auto* bTV = b->getAs<TypeVariableType>()) {
    Type bTVSubst = getSubstitution(bTV);
    // If bTV has a subst, unify the subst with a.
    // else, use a as the subst for bTV
    if (bTVSubst)
      return unify(a, bTVSubst, comparator);
    setSubstitution(bTV, a, false);
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
	// Note: getRValue is not desired here
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

Type Sema::createNewTypeVariable() {
  // TODO: Allocate that in a custom allocator

  // Create a new entry in the substitutions array for this 
  // new TypeVariable.
  typeVarsSubsts_.push_back(nullptr);
  return TypeVariableType::create(ctxt_, tyVarsCounter_++);
}

void Sema::resetTypeVariables() {
  tyVarsCounter_ = 0;
  typeVarsSubsts_.clear();
  // TODO: Reset the allocator here
}

Type Sema::getSubstitution(TypeVariableType* tyVar, bool recursively) {
  auto num = tyVar->getNumber();
  assert(num < typeVarsSubsts_.size() && "out-of-range");
  Type sub = typeVarsSubsts_[num];
  if(sub && recursively) {
    if(auto* tv = sub->getAs<TypeVariableType>())
      return getSubstitution(tv, true);
  }  
  return sub;
}

void Sema::setSubstitution(TypeVariableType* tyVar, Type subst, 
                           bool allowOverride) {
  auto num = tyVar->getNumber();
  assert(num < typeVarsSubsts_.size() && "out-of-range");
  TypeBase*& cur = typeVarsSubsts_[num];             
  
  if((!cur) || (cur && allowOverride))
    cur = subst.getPtr();
  else
    assert("A substitution already exists and it can't be overriden");
}
