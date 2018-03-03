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
			return isBasic(t) || (t == TypeIndex::VarRef);
		}
	}
	namespace Traits
	{
		template <typename T>
		struct typeIndex
		{
			constexpr static std::size_t index = TypeIndex::InvalidIndex;
		};
		// Specializations
		// monostate
		template<>
		struct typeIndex<NullType>
		{
			constexpr static std::size_t index = TypeIndex::Null_Type;
		};
		// int
		template<>
		struct typeIndex<IntType>
		{
			constexpr static std::size_t index = TypeIndex::basic_Int;
		};
		// float
		template<>
		struct typeIndex<float>
		{
			constexpr static std::size_t index = TypeIndex::basic_Float;
		};
		// char
		template<>
		struct typeIndex<CharType>
		{
			constexpr static std::size_t index = TypeIndex::basic_Char;
		};
		// string
		template<>
		struct typeIndex<std::string>
		{
			constexpr static std::size_t index = TypeIndex::basic_String;
		};
		// bool
		template<>
		struct typeIndex<bool>
		{
			constexpr static std::size_t index = TypeIndex::basic_Bool;
		};
		// monostate
		template<>
		struct typeIndex<Moonshot::var::VariableReference>
		{
			constexpr static std::size_t index = TypeIndex::VarRef;
		};
		// FoxValue traits class, to use with templated functions.
		// This class only operates on Indexes, and returns the index in the FoxValue of the type.
		template <typename T>
		struct FoxValueTraits : public typeIndex<T>
		{
			using typeIndex<T>::index;

			constexpr static bool is_basic = IndexUtils::isBasic(index);
			constexpr static bool is_arithmetic = IndexUtils::isArithmetic(index);
		};
	}
}