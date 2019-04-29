//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BuiltinsTypes.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/BuiltinsTypes.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"

using namespace fox;

//----------------------------------------------------------------------------//
// Implementation
//----------------------------------------------------------------------------//
namespace {
  using FnTyParam = FunctionTypeParam;

  template<typename Ty>
  struct TypeConverter {
    static Type get(ASTContext&) {
      static_assert(false, "This type is not supported by the Fox FFI!");
    }
  };
  
  template<typename ... Args> 
  struct ParamConverter {
    static void add(ASTContext&, SmallVectorImpl<FnTyParam>&) {}
  };

  template<typename First, typename ... Args> 
  struct ParamConverter<First, Args...> {
    static void add(ASTContext& ctxt, SmallVectorImpl<FnTyParam>& params) {
      params.emplace_back(TypeConverter<First>::get(ctxt), /*isMut*/ false);
      ParamConverter<Args...>::add(ctxt, params);
    }
  };

  #define TYPE_CONVERSION(TYPE, GET_IMPL) template<> struct TypeConverter<TYPE>\
    { static Type get(ASTContext& ctxt) { GET_IMPL; } }
  TYPE_CONVERSION(void,       return PrimitiveType::getVoid(ctxt));
  TYPE_CONVERSION(FoxInt,     return PrimitiveType::getInt(ctxt));
  TYPE_CONVERSION(FoxDouble,  return PrimitiveType::getDouble(ctxt));
  TYPE_CONVERSION(bool,       return PrimitiveType::getBool(ctxt));
  TYPE_CONVERSION(FoxChar,    return PrimitiveType::getChar(ctxt));
  #undef TYPE_CONVERSION

  template<typename Rtr, typename ... Args>
  Type getFoxTypeOfFunc(ASTContext& ctxt, Rtr(*)(Args...)) {
    Type returnType = TypeConverter<Rtr>::get(ctxt);
    SmallVector<FnTyParam, 4> params;
    ParamConverter<Args...>::add(ctxt, params);
    return FunctionType::get(ctxt, params, returnType);
  }
}

//----------------------------------------------------------------------------//
// Interface
//----------------------------------------------------------------------------//

Type fox::getTypeOfBuiltin(ASTContext& ctxt, BuiltinID id) {
  switch (id) {
    #define BUILTIN(FUNC, FOX)\
      case BuiltinID::FUNC:\
        return getFoxTypeOfFunc(ctxt, builtin::FUNC);
    #include "Fox/Common/Builtins.def"
    default:
      fox_unreachable("unknown BuiltinID");
  }
}
