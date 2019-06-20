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
#include "BuiltinKinds.hpp"
#include "FoxAny.hpp"
#include <string>

namespace fox {
  class VM;
  class StringObject;
  class ArrayObject;

  template<typename Ty>
  struct BuiltinFnArgTypeTrait { static constexpr bool ignored = false; };

  #define IGNORED_ARG_TYPE(TYPE) template<> \
  struct BuiltinFnArgTypeTrait<TYPE> { static constexpr bool ignored = true;  };
  #include "Builtins.def"

  namespace builtin {
    namespace util {
      // FIXME: These builtins all return strings, so when they're used
      // copies are made. Could this be made more efficient?

      /// Converts an integer \p value to a string
      std::string to_string(FoxInt value);
      /// Converts a double \p value to a string
      std::string to_string(FoxDouble value);
      /// Converts a bool \p value to a string
      std::string to_string(bool value);
      /// Converts a char \p value to a string
      std::string to_string(FoxChar value);
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

    /// \returns the nth codepoint (FoxChar) of a string.
    FoxChar getChar(VM& vm, StringObject* str, FoxInt n);

    /// inserts \p element in \p array
    void arrAppend(ArrayObject* arr, FoxAny elem);

    /// \returns the size of \p arr
    FoxInt arrSize(ArrayObject* arr);

    /// \returns the element at index \p n of \p arr
    FoxAny arrGet(VM& vm, ArrayObject* arr, FoxInt n);

    /// sets the element at index \p n of \p arr to \p val
    /// \returns \p val
    /// NOTE: This returns \p val because it makes things easier
    /// in BCGen.
    FoxAny arrSet(VM& vm, ArrayObject* arr, FoxInt n, FoxAny val);

    /// Removes the last element of \p arr
    void arrPop(ArrayObject* arr);

    /// \returns the first element of \p arr
    FoxAny arrFront(ArrayObject* arr);

    /// \returns the last element of \p arr
    FoxAny arrBack(ArrayObject* arr);

    /// Removes every element inside arr
    void arrReset(ArrayObject* arr);
  }
}