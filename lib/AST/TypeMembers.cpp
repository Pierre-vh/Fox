//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : TypeMembers.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/TypeMembers.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

const char* fox::to_string(TypeMemberKind value) {
  switch (value) {
    #define ANY_MEMBER(ID)\
      case TypeMemberKind::ID: return #ID;
    #include "Fox/AST/TypeMembers.def"
    default:
      fox_unreachable("unknown TypeBuiltinKind");
  }
}

std::ostream& fox::operator<<(std::ostream& os, TypeMemberKind value) {
  return os << to_string(value);
}