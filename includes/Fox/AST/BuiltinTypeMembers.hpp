//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BuiltinTypeMembers.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Contains the "BuiltinTypeMemberKind" enum and related functions.
//----------------------------------------------------------------------------//

#pragma once
 
#include <cstdint>
#include <string>
#include <iosfwd>

namespace fox {
  /// Kinds of Builtin Type Members.
  enum class BuiltinTypeMemberKind : std::uint16_t {
    #define ANY_MEMBER(ID) ID,
    #define MEMBER_RANGE(ID, FIRST, LAST)\
      First_##ID = FIRST, Last_##ID = LAST,
    #include "BuiltinTypeMembers.def"
  };

  /// \returns true if \p kind is a kind of String builtin
  bool isStringBuiltin(BuiltinTypeMemberKind kind) {
    return (kind >= BuiltinTypeMemberKind::First_StringMember) &&
           (kind <= BuiltinTypeMemberKind::Last_StringMember);
  }

  /// Converts a BuiltinTypeMemberKind to a human-readable string.
  const char* to_string(BuiltinTypeMemberKind value);
  std::ostream& operator<<(std::ostream& os, BuiltinTypeMemberKind value);
}