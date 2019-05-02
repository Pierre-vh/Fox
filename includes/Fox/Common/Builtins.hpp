//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Builtins.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the builtin functions of Fox.
//----------------------------------------------------------------------------//

#pragma once

#include "FoxTypes.hpp"
#include "BuiltinID.hpp"
#include <type_traits>

namespace fox {
  class VM;

  template<typename Ty>
  struct BuiltinFnArgTypeTrait { static constexpr bool ignored = false; };

  #define IGNORED_ARG_TYPE(TYPE) template<> \
  struct BuiltinFnArgTypeTrait<TYPE> { static constexpr bool ignored = true;  };
  #include "Builtins.def"

  namespace builtin {
    /// Prints an integer to stdout
    void printInt(FoxInt value);

    /// Prints a boolean to stdout
    void printBool(bool value);

    /// Prints a character to stdout
    void printChar(FoxChar ch);

    /// Prints a double to stdout
    void printDouble(FoxDouble value);
  }
}