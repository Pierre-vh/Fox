//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : TypeMembers.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Contains the "TypeBuiltinKind" enum and related functions.
//----------------------------------------------------------------------------//

#pragma once
 
#include <cstdint>
#include <string>
#include <iosfwd>

namespace fox {
  /// ID of members of builtin types.
  enum class TypeMemberKind : std::uint16_t {
    #define ANY_MEMBER(ID) ID,
    #define MEMBER_RANGE(ID, FIRST, LAST)\
      First_##ID = FIRST, Last_##ID = LAST,
    #include "TypeMembers.def"
  };

  /// \returns true if \p kind is a kind of String builtin
  bool isStringBuiltin(TypeMemberKind kind) {
    return (kind >= TypeMemberKind::First_StringMember) &&
           (kind <= TypeMemberKind::Last_StringMember);
  }

  /// Converts a TypeBuiltinKind to a human-readable string.
  const char* to_string(TypeMemberKind value);
  std::ostream& operator<<(std::ostream& os, TypeMemberKind value);
}