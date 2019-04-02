//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : FoxTypes.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file declares type aliases for various Fox types.
//
// The reason why I added theses aliases is because some types have
// multiple variants/widths, so one might be tempted to write 'float'
// instead of 'double', or 'unsigned' instead of 'std::int64_t', or even
// 'char' or 'wchar_t' instead of 'char32_t'. 
// Theses aliases ensure consistency when handling Fox values and avoid
// any problem related to implicit type conversions
//----------------------------------------------------------------------------//

#pragma once

#include <cstdint>

namespace fox {
  /// 64 Bit signed int.
  using FoxInt = std::int64_t;

  /// 32 bits Unicode Code Point
  using FoxChar = char32_t;

  /// 64 Bits Double Precision floating point number.
  using FoxDouble = double;

  /* No type alias for bool because it's just a 'bool' */
  
  /* No type alias for string because we use string_view or std::string
     depending on the situation */
}
