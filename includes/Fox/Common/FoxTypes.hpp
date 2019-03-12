//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : FoxTypes.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file declares type aliases for various Fox types to ensure consistency
// when handling values coming from Fox source code. Currently, 3 type aliases
// are used:
//    - FoxInt for the 64 bit signed integer
//    - FoxDouble for the double-precision (64 bits) floating point value
//    - FoxChar for the 32 bit unsigned integer/char
//
// The reason why I added theses aliases is because theses types have
// multiple variants/widths, so one might be tempted to write 'float'
// instead of 'double', or 'unsigned' instead of 'std::int64_t', or even
// 'char' or 'wchar_t' instead of 'char32_t'. 
// Theses aliases ensure consistency when handling Fox values and avoid
// any problem related to type inconsistencies.
//
// There are is type aliases for bool because it's just a classic boolean,
// and there isn't a type alias for string either because we use
// std::string/string_view depending on the situation. Theses type aliases
// would be useless and just confusing in my opinion.
//----------------------------------------------------------------------------//

#pragma once

#include <cstdint>

namespace fox {
  // 64 Bit signed int.
  using FoxInt = std::int64_t;

  // 32 bits Unicode Code Point
  using FoxChar = char32_t;

  // 64 Bits Double Precision floating point number.
  using FoxDouble = double;

  // No type alias for bool because it's just a 'bool'.
  
  // No type alias for string because we use string_view or std::string
  // depending on the situation.
}
