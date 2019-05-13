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
#include <string>

namespace fox {
  class VM;
  class StringObject;

  template<typename Ty>
  struct BuiltinFnArgTypeTrait { static constexpr bool ignored = false; };

  #define IGNORED_ARG_TYPE(TYPE) template<> \
  struct BuiltinFnArgTypeTrait<TYPE> { static constexpr bool ignored = true;  };
  #include "Builtins.def"

  namespace builtin {
    namespace detail {
      /// Helper functions for builtins

      /// \returns \p value converted as a string with full precision
      std::string foxDoubleTostring(FoxDouble value);
    }

    /// Prints an integer to stdout
    void printInt(FoxInt value);

    /// Prints a boolean to stdout
    void printBool(bool value);

    /// Prints a character to stdout
    void printChar(FoxChar ch);

    /// Prints a double to stdout
    void printDouble(FoxDouble value);

    /// Prints a string to stdout
    void printString(StringObject* str);

    /// Converts a char to a string
    StringObject* charToString(VM& vm, FoxChar value);

    /// Converts an integer to a string
    StringObject* intToString(VM& vm, FoxInt value);

    /// Converts a double to a string
    StringObject* doubleToString(VM& vm, FoxDouble value);

    /// Converts a boolean to a string
    StringObject* boolToString(VM& vm, bool value);

    /// Concatenates 2 strings together, producing a new string
    /// \returns a new string containing LHS and RHS.
    StringObject* strConcat(VM& vm, StringObject* lhs, StringObject* rhs);

    /// Concatenates 2 chars together
    /// \returns a new string that contains LHS and RHS.
    StringObject* charConcat(VM& vm, FoxChar lhs, FoxChar rhs);

    /// \returns the length of \p str in UTF8 Codepoints.
    FoxInt strLength(StringObject* str);

    /// \returns the size of \p str in bytes
    FoxInt strNumBytes(StringObject* str);
  }
}