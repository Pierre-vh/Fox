//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : SemaBuiltins.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  Contains the functions responsible for resolving references to members of
//  builtin types.
//----------------------------------------------------------------------------//

#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/BuiltinTypeMembers.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/string_view.hpp"

using namespace fox;

namespace {
  /// Searches for string builtin members with the identifier "id"
  void lookupStringMember(SmallVectorImpl<BuiltinTypeMemberKind>& results,
                          Identifier id) {
    // FIXME: A chain of ifs is suboptimal (but it shouldn't really be a
    //        problem here due to the relatively small amount of entries)
    //        lookupArrayMember below has the same problem.
    string_view str = id.getStr();
    #define STRING_MEMBER(ID, FOX)\
      if(str == #FOX) results.push_back(BuiltinTypeMemberKind::ID);
    #include "Fox/AST/BuiltinTypeMembers.def"
  }

  /// Searches for array builtin members with the identifier "id"
  void lookupArrayMember(SmallVectorImpl<BuiltinTypeMemberKind>& results,
                         Identifier id) {
    string_view str = id.getStr();
    #define ARRAY_MEMBER(ID, FOX)\
      if(str == #FOX) results.push_back(BuiltinTypeMemberKind::ID);
    #include "Fox/AST/BuiltinTypeMembers.def"
  }

  /// Helps creating BuiltinMemberRefExprs.
  struct BuiltinMemberRefBuilder {
    ASTContext& ctxt;
    Type voidType, intType;

    BuiltinMemberRefBuilder(ASTContext& ctxt) : ctxt(ctxt) {
      voidType = VoidType::get(ctxt);
      intType = IntegerType::get(ctxt);
    }

    /// Builds a BuiltinMemberRefExpr of kind \p builtinKind from \p ude
    BuiltinMemberRefExpr* build(UnresolvedDotExpr* ude, 
                                BuiltinTypeMemberKind builtinKind) {
      auto expr = BuiltinMemberRefExpr::create(ctxt, ude, builtinKind);
      expr->setType(getType(builtinKind, expr->getBase()->getType()));
      // Currently, members of builtin types are always methods (functions)
      expr->setIsMethod(); 
      return expr;
    }

    /// \returns the type of a builtin type member \p builtinKind used in
    /// on a type \p baseType
    Type getType(BuiltinTypeMemberKind builtinKind, Type baseType) {
      switch (builtinKind) { 
        default: fox_unreachable("unknown BuiltinTypeMemberKind");
        #define ARRAY_MEMBER(ID, FOX) case BuiltinTypeMemberKind::ID:\
          return getTypeOf##ID(baseType->castTo<ArrayType>());
        #define STRING_MEMBER(ID, FOX)\
          case BuiltinTypeMemberKind::ID: return getTypeOf##ID();
        #include "Fox/AST/BuiltinTypeMembers.def"
      }
    }

    //------------------------------------------------------------------------//
    // Array Members
    //------------------------------------------------------------------------//

    /// array.append is a function of type '(elementType) -> void'
    Type getTypeOfArrayAppend(ArrayType* baseType) {
      return FunctionType::get(ctxt, {baseType->getElementType()}, voidType);
    }

    /// array.size is a function of type '() -> int' 
    Type getTypeOfArraySize(ArrayType*) {
      return FunctionType::get(ctxt, {}, intType);
    }

    /// array.pop is a function of type '() -> void'
    Type getTypeOfArrayPop(ArrayType*) {
      return FunctionType::get(ctxt, {}, voidType);
    }

    /// array.front is a function of type '() -> elementType'
    Type getTypeOfArrayFront(ArrayType* baseType) {
      return FunctionType::get(ctxt, {}, baseType->getElementType());
    }

    /// array.back is a function of type '() -> elementType'
    Type getTypeOfArrayBack(ArrayType* baseType) {
      return FunctionType::get(ctxt, {}, baseType->getElementType());
    }

    //------------------------------------------------------------------------//
    // String Members
    //------------------------------------------------------------------------//
    
    /// string.size is a function of type '() -> int'
    Type getTypeOfStringSize() {
      return FunctionType::get(ctxt, {}, intType);
    }

    /// string.numBytes is a function of type '() -> int'
    Type getTypeOfStringNumBytes() {
      return FunctionType::get(ctxt, {}, intType);
    }
  };
}

/// Attempts to resolve a reference to a member of a builtin type.
/// \returns nullptr if the member couldn't be resolved (= unknown),
/// else returns the resolved expression.
BuiltinMemberRefExpr* Sema::resolveBuiltinTypeMember(UnresolvedDotExpr* expr) {
  Identifier memberID = expr->getMemberIdentifier();

  // Lookup the builtin member
  SmallVector<BuiltinTypeMemberKind, 4> results;
  Type baseType = expr->getBase()->getType();
  if(baseType->isStringType())
    lookupStringMember(results, memberID);
  else if(baseType->isArrayType())
    lookupArrayMember(results, memberID);
  // No other builtin type have members, so bail directly.
  else 
    return nullptr;

  // If there's no result, the member doesn't exist.
  if(results.size() == 0)
    return nullptr;

  // There should be only one result since we don't support overloading members
  // of builtin types.
  assert((results.size() == 1) && "unsupported overloaded builtin type member");

  // Build the BuiltinMemberRefExpr and return.
  return BuiltinMemberRefBuilder(ctxt).build(expr, results[0]);
}