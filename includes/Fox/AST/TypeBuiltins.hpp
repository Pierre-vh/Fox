//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : TypeBuiltins.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Contains the "TypeBuiltinKind" enum and related functions.
//----------------------------------------------------------------------------//

#pragma once
 
#include <cstdint>
#include <string>
#include <iosfwd>

namespace fox {
  /// Kinds of members of builtin types.
  enum class TypeBuiltinKind : std::uint16_t {
    #define TYPE_BUILTIN_METHOD(ID, FOX) ID,
    #define TYPE_BUILTIN_RANGE(ID, FIRST, LAST)\
      First_##ID = FIRST, Last_##ID = LAST,
    #include "TypeBuiltins.def"
  };

  /// \returns true if \p kind is a kind of String builtin
  bool isStringBuiltin(TypeBuiltinKind kind) {
    return (kind >= TypeBuiltinKind::First_StringBuiltin) &&
           (kind <= TypeBuiltinKind::Last_StringBuiltin);
  }

  /// Converts a TypeBuiltinKind to a human-readable string.
  const char* to_string(TypeBuiltinKind value);
  std::ostream& operator<<(std::ostream& os, TypeBuiltinKind value);
}