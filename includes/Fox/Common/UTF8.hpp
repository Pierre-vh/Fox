//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : UTF8.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains some general utility functions related to UTF-8 
// codepoints (FoxChar) handling and UTF-8 string handling.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/Common/FoxTypes.hpp"
#include <string>

namespace fox {
  /// Inserts a FoxChar at the end of a std::string.
  /// Can be used to convert a unicode codepoint (FoxChar) to a std::string.
  /// \param ch the char to convert
  /// \param dest the destination string
  void appendFoxChar(FoxChar ch, std::string& dest);
}

