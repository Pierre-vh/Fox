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

// Typedefs for every type that Fox supports. Theses should only
// be used when you want to make it clear to the reader that you'll be
// containing a value from the Fox source code. Don't use theses in the wild
// to store arbitrary values.

// 64 Bit signed int.
typedef std::int64_t FoxInt;

// 32 bits Unicode Code Point
typedef char32_t FoxChar;

// 32 Bits Single Precision floating point number.
typedef float FoxFloat;

// String value
typedef std::string FoxString;

// Boolean value
typedef bool FoxBool;