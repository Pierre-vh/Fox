//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
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
    Type aTVSubst = aTV->getSubst();
    // TypeVariable = TypeVariable
    //  Both are TypeVariable, check if they have substitutions
    if (auto* bTV = b->getAs<TypeVariableType>()) {
      Type bTVSubst = bTV->getSubst();
      // Both have a sub
      if (aTVSubst && bTVSubst)
        return unify(aTVSubst, bTVSubst, comparator);
      // A has a sub, B doesn't
      if (aTVSubst) {
        // Set B's substitution to A.
        bTV->assignSubst(aTV);
        return true;
      }
      // B has a sub, A doesn't
      if (bTVSubst) {
        // Set A's substitution to B
        aTV->assignSubst(bTV);
        return true;
      }
      // None of them have a substitution: set the
      // substitution of aTV to bTV. 
      // 
      // e.g. if aTV = $Ta and bTV = $Tb, then
      // $Ta = $Tb causes $Ta to be bound to $Tb.
      aTV->assignSubst(bTV);
      return true;
    }
    // TypeVariable = (Not TypeVariable)
    else {
      // If aTV has a subst, unify the subst with b.
      // else, use b as the subst for aTV
      if (aTVSubst)
        return unify(aTVSubst, b, comparator);
      aTV->assignSubst(b);
      return true;
    }
  }
  // (Not TypeVariable) = TypeVariable
  else if (auto* bTV = b->getAs<TypeVariableType>()) {
    Type bTVSubst = bTV->getSubst();
    // If bTV has a subst, unify the subst with a.
    // else, use a as the subst for bTV
    if (bTVSubst)
      return unify(a, bTVSubst, comparator);
    bTV->assignSubst(a);
    return true;
  }
  // Can't unify.
  return false;
}

Sema::TypePair Sema::unwrapAll(Type a, Type b) {
  assert(a && b && "args cannot be null");
  // Ignore LValues 
  a = a->getRValue();
  b = b->getRValue();

  // Unwrap arrays
  while (true) {
    if (a->is<ArrayType>() && b->is<ArrayType>()) {
      a = a->castTo<ArrayType>()->getElementType();
      b = b->castTo<ArrayType>()->getElementType();
      assert(a && 
        "'a' had a null element type!");
      assert(b && 
        "'b' had a null element type!");
    }
    else break;
  }

  return {a, b};
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
        return type->getSubstRecursively();
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
  return TypeVariableType::create(ctxt_, tyVarsCounter_++);
}

void Sema::resetTypeVariables() {
  tyVarsCounter_ = 0;
  // TODO: Reset the allocator here.
}