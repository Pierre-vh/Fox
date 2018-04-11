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
// normal int ! To solve this, they would both use "IntType" !
////------------------------------------------------------////

#pragma once

// Typedefs for every type, except strings and bools, since they'll always stay that way.
// Typedefs are only useful for type that might change later, or sort of complex typenames 
// e.g. "CharType" is much more expressive and nice to read than char32_t, imo.

// 64 Bit signed int.
typedef int64_t			IntType;

// 32 bits unicode Char
typedef char32_t		CharType;

// 32 Bits Single Precision floating point number. Might become a 64 Bits one later if needed.
typedef float			FloatType;

// A Void, Empty Typename.
typedef std::monostate	VoidType;

namespace Moonshot
{
	namespace TypeLimits
	{
		static constexpr IntType IntType_MAX = (std::numeric_limits<IntType>::max)();
		static constexpr IntType IntType_MIN = (std::numeric_limits<IntType>::min)();

		static constexpr CharType CharType_MAX = (std::numeric_limits<CharType>::max)();
		static constexpr CharType CharType_MIN = (std::numeric_limits<CharType>::min)();

		static constexpr FloatType FloatType_MAX = (std::numeric_limits<FloatType>::max)();
		static constexpr FloatType FloatType_MIN = (std::numeric_limits<FloatType>::min)();
	};
}