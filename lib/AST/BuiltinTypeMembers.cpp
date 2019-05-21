//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BuiltinTypeMembers.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/AST/BuiltinTypeMembers.hpp"
#include "Fox/Common/Errors.hpp"

using namespace fox;

const char* fox::to_string(BuiltinTypeMemberKind value) {
  switch (value) {
    #define ANY_MEMBER(ID)\
      case BuiltinTypeMemberKind::ID: return #ID;
    #include "Fox/AST/BuiltinTypeMembers.def"
    default:
      fox_unreachable("unknown TypeBuiltinKind");
  }
}

std::ostream& fox::operator<<(std::ostream& os, BuiltinTypeMemberKind value) {
  return os << to_string(value);
}