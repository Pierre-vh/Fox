//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : TypeBuiltins.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/TypeBuiltins.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

const char* fox::to_string(TypeBuiltinKind value) {
  switch (value) {
    #define TYPE_BUILTIN_METHOD(ID, FOX)\
      case TypeBuiltinKind::ID: return #ID;
    #include "Fox/AST/TypeBuiltins.def"
    default:
      fox_unreachable("unknown TypeBuiltinKind");
  }
}

std::ostream& fox::operator<<(std::ostream& os, TypeBuiltinKind value) {
  return os << to_string(value);
}