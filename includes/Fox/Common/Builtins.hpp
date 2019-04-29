//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Builtins.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the BuiltinID enum and the declaration of the builtin 
// functions available in Fox.
//----------------------------------------------------------------------------//

#pragma once

#include <cstdint>
#include "FoxTypes.hpp"

namespace fox {
  /// The underlying type of the BuiltinID enum
  using builtin_id_t = std::uint8_t;

  /// Enumeration representing every kind of builtin
  enum class BuiltinID : builtin_id_t {
    invalid,
    #define BUILTIN(FUNC, ID) FUNC,
    #include "Builtins.def"
  };

  const char* to_string(BuiltinID id);

  namespace builtin {
    /// Prints an integer to stdout
    void printInt(FoxInt value);

    /// Prints a boolean to stdout
    void printBool(bool value);
  }
}