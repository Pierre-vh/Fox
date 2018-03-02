////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : FVTypeTraits.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the struct "TypeTrait_FVal" which is used with std::visit
// to determine the category of a templated type in a FVal. example : TypeTrait_FVal<T>::is_basic
////------------------------------------------------------////

#pragma once
#include "Types.hpp"
#include <type_traits>

namespace Moonshot::TypeUtils
{
	// Index utility function
	inline constexpr bool isBasic(const std::size_t& t)
	{
		switch (t)
		{
			case Types::basic_Int:
			case Types::basic_Float:
			case Types::basic_Char:
			case Types::basic_Bool:
			case Types::basic_String:
				return true;
			default:
				return false;
		}
	}
	inline constexpr bool isArithmetic(const std::size_t& t)
	{
		return (t == Types::basic_Float) || (t == Types::basic_Bool) || (t == Types::basic_Int);
	}
	
	inline constexpr bool isValue(const std::size_t& t)
	{
		return isBasic(t) || (t == Types::basic_VarRef);
	}

	template <typename T>
	struct typeIndex
	{
		constexpr static std::size_t index = Types::InvalidIndex;
	};
	// Specializations
	// monostate
	template<>
	struct typeIndex<NullType>
	{
		constexpr static std::size_t index = Types::basic_Null;
	};
	// int
	template<>
	struct typeIndex<IntType>
	{
		constexpr static std::size_t index = Types::basic_Int;
	};
	// float
	template<>
	struct typeIndex<float>
	{
		constexpr static std::size_t index = Types::basic_Float;
	};
	// char
	template<>
	struct typeIndex<CharType>
	{
		constexpr static std::size_t index = Types::basic_Char;
	};
	// string
	template<>
	struct typeIndex<std::string>
	{
		constexpr static std::size_t index = Types::basic_String;
	};
	// bool
	template<>
	struct typeIndex<bool>
	{
		constexpr static std::size_t index = Types::basic_Bool;
	};
	// monostate
	template<>
	struct typeIndex<Moonshot::var::varRef>
	{
		constexpr static std::size_t index = Types::basic_VarRef;
	};
	// FValue traits class, to use with templated functions.
	template <typename T>
	struct TypeTrait_FVal : public typeIndex<T>
	{
		using typeIndex<T>::index;

		constexpr static bool is_basic = isBasic(index);
		constexpr static bool is_arithmetic = isArithmetic(index);
	};
}