//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BuiltinID.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the BuiltinID enum as well as some builtin-related
// helper functions that do not need the builtin function to be visible
// in the header (e.g. isPublic, hasNonVoidReturnType)
//----------------------------------------------------------------------------//

#pragma once

#include <cstdint>
#include <iosfwd>

namespace fox {
  /// The underlying type of the BuiltinID enum
  using builtin_id_t = std::uint8_t;

  /// Enumeration representing every kind of builtin function.
  enum class BuiltinID : builtin_id_t {
    #define BUILTIN(FUNC) FUNC,
    #define BUILTIN_RANGE(NAME, FIRST, LAST)\
      First_##NAME = FIRST, Last_##NAME = LAST,
    #include "Builtins.def"
  };

  /// \returns true if the builtin with id \p id is a public one.
  inline bool isPublic(BuiltinID id) {
    return (id >= BuiltinID::First_Public) && (id <= BuiltinID::Last_Public);
  }

  /// \returns true if the builtin with id \p id has a non-void return type.
  /// For example, this is used by the bytecode generator to choose between
  /// Call and CallVoid to call the builtin.
  bool hasNonVoidReturnType(BuiltinID id);

  const char* to_string(BuiltinID id);
  std::ostream& operator<<(std::ostream& os, BuiltinID id);
}