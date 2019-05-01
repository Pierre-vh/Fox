//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BuiltinID.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the BuiltinID enum.
//----------------------------------------------------------------------------//

#pragma once

#include <cstdint>
#include <iosfwd>

namespace fox {
  /// The underlying type of the BuiltinID enum
  using builtin_id_t = std::uint8_t;

  /// Enumeration representing every kind of builtin
  enum class BuiltinID : builtin_id_t {
    #define BUILTIN(FUNC, ID) FUNC,
    #include "Builtins.def"
  };

  const char* to_string(BuiltinID id);
  std::ostream& operator<<(std::ostream& os, BuiltinID id);
}