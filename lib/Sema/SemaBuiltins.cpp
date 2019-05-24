//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : SemaBuiltins.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  Contains the functions responsible for resolving references to builtin
//  members of builtin types
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
    // FIXME: A chain of "if"s is suboptimal, but the number of builtins is
    //        currently really small so it shouldn't be a problem.
    //        A cached lookup map would be ideal though.
    //        See lookupArrayMember for a similar FIXME.
    using BTMK = BuiltinTypeMemberKind;
    string_view str = id.getStr();
    #define STRING_MEMBER(ID, FOX) if(str == #FOX) results.push_back(BTMK::ID);
    #include "Fox/AST/BuiltinTypeMembers.def"
  }

  /// Searches for array builtin members with the identifier "id"
  void lookupArrayMember(SmallVectorImpl<BuiltinTypeMemberKind>& results,
                          Identifier id) {
    // FIXME: A chain of "if"s is suboptimal, but the number of builtins is
    //        currently really small so it shouldn't be a problem.
    //        A cached lookup map would be ideal though.
    using BTMK = BuiltinTypeMemberKind;
    string_view str = id.getStr();
    #define ARRAY_MEMBER(ID, FOX) if(str == #FOX) results.push_back(BTMK::ID);
    #include "Fox/AST/BuiltinTypeMembers.def"
  }

  /// \returns the type of a builtin string member \p kind 
  Type getTypeOfBuiltinStringMember(ASTContext& ctxt, 
                                    BuiltinTypeMemberKind kind) {
    assert(isStringBuiltin(kind) && "wrong function");
    using BTMK = BuiltinTypeMemberKind;

    switch (kind) {
      default: fox_unreachable("Unknown String Builtin Kind");
      case BTMK::StringSize:
      case BTMK::StringNumBytes:
        /// string.numBytes and .size are function of type '() -> int'
        return FunctionType::get(ctxt, {}, IntegerType::get(ctxt));
    }
  }

  /// \returns the type of a builtin array member \p kind 
  Type getTypeOfBuiltinArrayMember(ASTContext& ctxt, BuiltinTypeMemberKind kind,
                                   Type baseType) {
    assert(isArrayBuiltin(kind) && "wrong function");
    using BTMK = BuiltinTypeMemberKind;

    ArrayType* baseArray = baseType->getRValue()->getAs<ArrayType>();
    assert(baseArray && "base isn't an array type?");
    Type elementType = baseArray->getElementType();

    switch (kind) {
      default: fox_unreachable("Unknown Array Builtin Kind");
      case BTMK::ArraySize:
        /// array.size is a function of type '() -> int' 
        return FunctionType::get(ctxt, {}, IntegerType::get(ctxt));
      case BTMK::ArrayAppend:
        /// array.append is a function of type '(elementType) -> void'
        return FunctionType::get(ctxt, {elementType}, VoidType::get(ctxt));
      case BTMK::ArrayPop:
        /// array.pop is a function of type '() -> void'
        return FunctionType::get(ctxt, {}, VoidType::get(ctxt));
      case BTMK::ArrayFront:
      case BTMK::ArrayBack:
        /// array.front and .back are functions of type '() -> elementType'
        return FunctionType::get(ctxt, {}, elementType);
    }
  }

  /// \returns the type of a BuiltinTypeMemberKind.
  /// Never nullptr.
  Type getTypeOfBuiltinMember(ASTContext& ctxt, BuiltinMemberRefExpr* expr) {
    Type baseType = expr->getBase()->getType();
    auto kind = expr->getBuiltinTypeMemberKind();

    if(isStringBuiltin(kind))
      return getTypeOfBuiltinStringMember(ctxt, kind);
    if(isArrayBuiltin(kind))
      return getTypeOfBuiltinArrayMember(ctxt, kind, baseType);
    fox_unreachable("unknown BuiltinTypeMemberKind category");
  }
}

/// Attempts to resolve a reference to a builtin member of a type.
/// \returns nullptr if the member couldn't be resolved (= unknown),
/// else returns the resolved expression.
BuiltinMemberRefExpr* 
Sema::resolveBuiltinMember(UnresolvedDotExpr* expr) {
  Identifier memberID = expr->getMemberIdentifier();
  // Lookup the builtin member
  SmallVector<BuiltinTypeMemberKind, 4> results;
  Type baseType = expr->getBase()->getType();
  if(baseType->isStringType())
    lookupStringMember(results, memberID);
  else if(baseType->isArrayType())
    lookupArrayMember(results, memberID);
  // No other type have builtin members.
  else return nullptr;

  // If there's no result, the builtin doesn't exist.
  if(results.size() == 0)
    return nullptr;

  // There should be only one result since we don't support overloading builtin
  // members.
  assert((results.size() == 1) && "unsupported overloaded builtin type member");

  // Create the resolved expression
  BuiltinTypeMemberKind builtinKind = results[0];
  auto resolved = BuiltinMemberRefExpr::create(ctxt, expr, builtinKind);
  // Set its type
  resolved->setType(getTypeOfBuiltinMember(ctxt, resolved));
  // Currently, builtins are always functions.
  resolved->setIsMethod();
  return resolved;
}