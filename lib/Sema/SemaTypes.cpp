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
    // Only allow exact equality
    return (a == b);
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
      // None of them have a substitution: set the
      // substitution of aTV to bTV. 
      // 
      // e.g. if aTV = $Ta and bTV = $Tb, then
      // $Ta = $Tb causes $Ta to be bound to $Tb.
      setSubstitution(aTV, bTV, false);
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
  // Can't unify.
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
  // Ignore LValues 
  auto uwA = a->getRValue();
  auto uwB = b->getRValue();
  // Unwrap arrays
  std::tie(uwA, uwB) = unwrapArrays(uwA, uwB);
  // If both changed, recurse, else, return.
  if ((uwA != a) && (uwB != b))
    return unwrapAll(uwA, uwB);
  return {uwA, uwB};
}

Type Sema::simplify(Type type) {
  class Impl : public TypeVisitor<Impl, Type> {
    public:
      ASTContext& ctxt;
      const Sema& sema;

      Impl(Sema& sema) : 
        sema(sema), ctxt(sema.getASTContext()) {}

      Type visitPrimitiveType(PrimitiveType* type) {
        return type;
      }

      Type visitArrayType(ArrayType* type) {
        if (Type elem = visit(type->getElementType())) {
          if (elem != type->getElementType())
            return ArrayType::get(ctxt, elem);
          return type;
        }
        return nullptr;
      }

      Type visitLValueType(LValueType* type) {
        if (Type elem = visit(type->getType())) {
          if (elem != type->getType())
            return ArrayType::get(ctxt, elem);
          return type;
        }
        return nullptr;
      }

      Type visitTypeVariableType(TypeVariableType* type) {
        // Just return the *real* substitution for that TypeVariable.
        // If there's none (nullptr), the visitors will all 
        // return nullptr too, notifying handleExprPre of the inference
        // failure.
        return sema.getSubstitution(type, /*recursively*/ true);
      }

      Type visitErrorType(ErrorType* type) {
        // Assert that we have emitted at least 1 error if
        // we have a ErrorType present in the hierarchy.
        return type;
      }

      Type visitFunctionType(FunctionType* type) {
        // Get return type
        Type returnType = visit(type->getReturnType());

        if (!returnType) return nullptr;
        // Get Param types
        SmallVector<Type, 4> paramTypes;
        for (auto param : type->getParamTypes()) {
          Type t = visit(param);
          if (!t) return nullptr;
          paramTypes.push_back(t);
        }
        // Recompute if needed
        if (!type->isSame(paramTypes, returnType))
          return FunctionType::get(ctxt, paramTypes, returnType);
        return type;
      }
  };
  assert(type && "arg is nullptr!");
  return type->hasTypeVariable() ? Impl(*this).visit(type) : type;
}

Type Sema::trySimplify(Type type) {
  if(Type simplified = simplify(type))
   return simplified;
  return type;
}

bool Sema::isWellFormed(Type type) {
  return !type->hasErrorType();
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

Type
Sema::getSubstitution(TypeVariableType* tyVar, bool recursively) const {
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
