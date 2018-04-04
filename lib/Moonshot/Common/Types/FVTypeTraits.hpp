////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : FVTypeTraits.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the struct "FoxValueTraits" which is used with std::visit
// to determine the category of a templated type in a FoxValue. example : FoxValueTraits<T>::is_basic
//
// This also contains various Index-based functions and utilities.
////------------------------------------------------------////

#pragma once
#include "Types.hpp"
#include <type_traits>

namespace Moonshot
{
	// Index utility function
	namespace IndexUtils
	{
		inline constexpr bool isBasic(const std::size_t& t)
		{
			switch (t)
			{
				case TypeIndex::basic_Int:
				case TypeIndex::basic_Float:
				case TypeIndex::basic_Char:
				case TypeIndex::basic_Bool:
				case TypeIndex::basic_String:
					return true;
				default:
					return false;
			}
		}
		inline constexpr bool isArithmetic(const std::size_t& t)
		{
			return (t == TypeIndex::basic_Float) || (t == TypeIndex::basic_Bool) || (t == TypeIndex::basic_Int);
		}

		inline constexpr bool isValue(const std::size_t& t)
		{
			return isBasic(t); // Currently only basic types exists, and they're all values :/
		}
	}
}