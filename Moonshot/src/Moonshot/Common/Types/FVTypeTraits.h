////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : FVTypeTraits.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the struct "fval_traits" which is used with std::visit
// to determine the category of a templated type in a FVal. example : fval_traits<T>::isBasic
////------------------------------------------------------////

#pragma once
#include <type_traits>

namespace Moonshot::fv_util
{
	// FValue traits class, to use with templated functions.
	template <typename T>
	struct fval_traits
	{
		constexpr static bool isBasic =
			std::is_same<T, int64_t>::value ||
			std::is_same<T, float>::value ||
			std::is_same<T, CharType>::value ||
			std::is_same<T, std::string>::value ||
			std::is_same<T, bool>::value
			;
		constexpr static bool isArithmetic = isBasic && !std::is_same<T, std::string>::value;

		constexpr static inline bool isEqualTo(const std::size_t& index) // Checks if T represent the same type as index
		{
			if constexpr(std::is_same<T, std::monostate>::value)
				return index == indexes::fval_null;
			else if constexpr(std::is_same<T, int64_t>::value)
				return index == indexes::fval_int;
			else if constexpr(std::is_same<T, float>::value)
				return index == indexes::fval_float;
			else if constexpr(std::is_same<T, CharType>::value)
				return index == indexes::fval_char;
			else if constexpr(std::is_same<T, std::string>::value)
				return index == indexes::fval_str;
			else if constexpr(std::is_same<T, bool>::value)
				return index == indexes::fval_bool;
			else if constexpr(std::is_same<T, var::varRef>::value)
				return index == indexes::fval_varRef;
			else
			{
				throw std::logic_error("Defaulted");
				return false;
			}
		}
	};
}