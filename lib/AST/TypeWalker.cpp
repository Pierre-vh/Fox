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
  /// TypeTraverse, the traverse class for Types.
  class TypeTraverse : public TypeVisitor<TypeTraverse, bool> {
    public:
      TypeTraverse(TypeWalker& walker) : walker(walker) {}

      TypeWalker &walker;

      bool doIt(Type type) {
        if (!walker.handleTypePre(type))
          return true;
        if (visit(type))
          return walker.handleTypePost(type);
        return false;
      }

      bool visitBasicType(BasicType*) {
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

      bool visitTypeVariableType(TypeVariableType*) {
        return true;
      }

      bool visitFunctionType(FunctionType* type) {
        if(Type rtr = type->getReturnType())
          if(!doIt(rtr)) return false;

        for(auto param : type->getParams()) {
          if(Type paramTy = param.getType())
            if(!doIt(paramTy)) return false;
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