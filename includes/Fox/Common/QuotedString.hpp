//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : QuotedString.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains an utility function used to print a string to a ostream,
// replacing characters such as '\n' by escapes sequences.
// This lets the caller print a string without messing up their formatting.
//----------------------------------------------------------------------------//

#pragma once

#include "string_view.hpp"
#include "FoxTypes.hpp"
#include <iosfwd>

namespace fox {
  /// Prints a quoted string, replacing formatting characters such as
  /// \n or \r with escape sequences
  /// \param str the string to print
  /// \param os the output stream
  /// \param delim optional the delimiter to use when printing the string
  ///        default is '\0' (none). 
  ///        If the delimiter is found within \p str, it will
  ///        also be printed with a backslash.
  void printQuotedString(string_view str, std::ostream& os, char delim = 0);

  /// Prints a quoted character, displaying it as an escape sequence if 
  /// it is a formatting character (\0, \n, etc)
  /// \param ch the char to print
  /// \param os the output stream
  /// \param delim optional the delimiter to use when printing the character
  ///        default is '\0' (none). 
  ///        If the delimiter is equal to \p ch, it'll be printed with a 
  ///        backslash
  void printQuotedChar(FoxChar ch, std::ostream& out, char delim = 0);
}