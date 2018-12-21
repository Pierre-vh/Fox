//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Typedefs.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file declares Typedefs for various types to ensure
// consistency between types. 
//
// For instance, it would be terrible if a class tried to store a int64_t, used by a literal token, in a
// normal int ! To solve this, they would both use "FoxInt" !
//----------------------------------------------------------------------------//

#pragma once

#include <string>
#include <cstddef>

namespace fox {
  // Type aliases for every type that Fox supports. Theses should only
  // be used when you want to make it clear that you'll be
  // storing a value which was found in the source code fed to the interpreter. 
  // Don't use theses in the wild to store arbitrary values.

  // For instance, a AST node that holds a constant (a literal) will use the typedef
  // when manipulating the value,  but your everyday boolean flag in a function shouldn't use it.

  // 64 Bit signed int.
  using FoxInt = std::int64_t;

  // 32 bits Unicode Code Point
  using FoxChar = char32_t;

  // 32 Bits Single Precision floating point number.
  using FoxFloat = float;
}
