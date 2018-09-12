////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Typedefs.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares Typedefs for various types to ensure
// consistency between types. 
//
// For instance, it would be terrible if a class tried to store a int64_t, used by a literal token, in a
// normal int ! To solve this, they would both use "FoxInt" !
////------------------------------------------------------////

#pragma once

#include <string>
#include <cstddef>

// Typedefs for every type, except strings and bools, since they'll always stay that way.
// Typedefs are only useful for type that might change later, or sort of complex typenames 
// e.g. "FoxChar" is much more expressive and nice to read than char32_t, imo.

// 64 Bit signed int.
typedef std::int64_t FoxInt;

// 32 bits unicode Char
typedef char32_t FoxChar;

// 32 Bits Single Precision floating point number. Might become a 64 Bits one later if needed.
typedef float FoxFloat;

// String value
typedef std::string FoxString;

// Boolean value
typedef bool FoxBool;