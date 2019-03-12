//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Typedefs.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file declares Typedefs for various types to ensure
// consistency when handling Fox values.
//----------------------------------------------------------------------------//

#pragma once

#include <string>
#include <cstddef>

namespace fox {
  // Type aliases for every type that Fox supports. Theses should only
  // be used when you want to make it clear that you'll be
  // storing a 'Fox Value' (a value entered by the user or something like that)

  // 64 Bit signed int.
  using FoxInt = std::int64_t;

  // 32 bits Unicode Code Point
  using FoxChar = char32_t;

  // 64 Bits Double Precision floating point number.
  using FoxDouble = double;

  // No type alias for bool because it's just a boolean.
  
  // No type alias for string because we usually use string_view
}
