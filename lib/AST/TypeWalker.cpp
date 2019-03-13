//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : TypeWalker.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/TypeWalker.hpp"
#include "Fox/AST/TypeVisitor.hpp"
#include "Fox/AST/Type.hpp"
#include "Fox/AST/Types.hpp"

using namespace fox;

namespace {
  // TypeTraverse, the traverse class for Types.
  class TypeTraverse : public TypeVisitor<TypeTraverse, bool> {
    TypeWalker &walker_;
    public:
      TypeTraverse(TypeWalker& walker) : walker_(walker) {}

      // doIt method for types
      bool doIt(Type type) {
        // Call the walker, abort if failed.
        if (!walker_.handleTypePre(type))
          return false;

        // Visit the children
        if (visit(type))
          // Call the walker (post)
          return walker_.handleTypePost(type);
        return false;
      }

      bool visitPrimitiveType(PrimitiveType*) {
        return true;
      }

      bool visitArrayType(ArrayType* type) {
        if (Type elem = type->getElementType())
          return doIt(elem);
        return true;
      }

      bool visitLValueType(LValueType* type) {
        if (Type ty = type->getType())
          return doIt(ty);
        return true;
      }

      bool visitErrorType(ErrorType*) {
        return true;
      }

      bool visitTypeVariableType(TypeVariableType*) {
        return true;
      }

      bool visitFunctionType(FunctionType* type) {
        if(Type rtr = type->getReturnType())
          if(!doIt(rtr)) return false;

        for(auto paramTy : type->getParamTypes()) {
          if(paramTy) {
            if(!doIt(paramTy)) return false;
          }
        }

        return true;
      }
  };
} // end anonymous namespace

bool TypeWalker::walk(Type type) {
  return TypeTraverse(*this).doIt(type);
}

bool TypeWalker::handleTypePre(Type) {
  return true;
}

bool TypeWalker::handleTypePost(Type) {
  return true;
}