////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : PtrVariant.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This class implements a Pointer variant. It just stores a void pointer,
// and a 8 bit discriminant. It can be queried to check which type is currently in use.
// Note that you must omit the pointer when using the class. 
// e.g. PtrVariant<int,float> not PtrVariant<int*,float*>
//
// TODO: Implement an alternative "intrusive" variant which stores the 
// discriminant in the low bits of the pointer.
////------------------------------------------------------////

#pragma once

#include <typeindex>
#include <type_traits>
#include <cstddef>
#include <cassert>

template <typename ... Args>
class PtrVariant
{
	private:
		using Idx = std::uint8_t;

		template<typename T>
		using hasType = std::disjunction<std::is_same<T, Args>...>;

		template<typename T>
		using enableIf_hasType = std::enable_if<hasType<T>::value, int>;

		using ThisType = PtrVariant<Args...>;
	public:
		// Assertion/Checks
		static_assert(is_unique<Args...>::value, "No duplicates allowed in the Pointer Variant.");

		// Constructors
		PtrVariant() : ptr_(nullptr), idx_(0)
		{

		}

		template<typename T, typename enableIf_hasType<T>::type = 0>
		PtrVariant(T* ptr) : ptr_(ptr), idx_(indexOf<T>::value)
		{

		}

		// Setter
		template<typename T, typename enableIf_hasType<T>::type = 0>
		void set(T* ptr)
		{
			ptr_ = ptr;
			idx_ = indexOf<T>::value;
		}

		// Getter that asserts the it's the correct type
		template<typename T, typename enableIf_hasType<T>::type = 0>
		T* get() const
		{
			assert((idx_ == indexOf<T>::value) && "Incorrect type!");
			return static_cast<T*>(ptr_);
		}

		// Getter that returns nullptr if it's not the correct type
		template<typename T, typename enableIf_hasType<T>::type = 0>
		T* getIf() const
		{
			if (idx_ == indexOf<T>::value)
				return static_cast<T*>(ptr_);
			return nullptr;
		}


		// Checking
		template<typename T, typename enableIf_hasType<T>::type = 0>
		bool is() const
		{
			return (idx_ == indexOf<T>::value);
		}

		// Operator overloads
		// Comparisons
		bool operator== (const ThisType& other) const
		{
			return ptr_ == other.ptr_;
		}

		template<typename T, typename enableIf_hasType<T>::type = 0>
		bool operator== (const T* other) const
		{
			return ptr_ == other;
		}

		bool operator!= (const ThisType& other) const
		{
			return ptr_ != other.ptr_;
		}

		template<typename T, typename enableIf_hasType<T>::type = 0>
		bool operator!= (const T* other) const
		{
			return (ptr_ != other);
		}

		// Assignement
		template<typename T, typename enableIf_hasType<T>::type = 0>
		ThisType& operator= (T* other)
		{
			set(other);
			return *this;
		}

		template<typename T, typename enableIf_hasType<T>::type = 0>
		ThisType& operator= (const ThisType& other) const
		{
			idx_ = other.idx_;
			ptr_ = other.ptr_;
		}
	private:
		// Check for uniqueness helpers
		template <typename T, typename... List>
		struct is_contained;

		template <typename T, typename Head, typename... Tail>
		struct is_contained<T, Head, Tail...>
		{
			static constexpr bool value = std::is_same<T, Head>::value || is_contained<T, Tail...>::value;
		};

		template <typename T>
		struct is_contained<T>
		{
			static constexpr bool value = false;
		};

		template <typename... List>
		struct is_unique;

		template <typename Head, typename... Tail>
		struct is_unique<Head, Tail...>
		{
			static constexpr bool value = !is_contained<Head, Tail...>::value && is_unique<Tail...>::value;
		};

		template <>
		struct is_unique<>
		{
			static constexpr bool value = true;
		};

		// Get the index of a type in the parameter pack
		template<typename...>
		struct index_impl;

		template<typename Req, typename ... Rest>
		struct index_impl<Req, Req, Rest...> : std::integral_constant<Idx, 0>
		{};

		template<typename Req, typename Other, typename ... Rest>
		struct index_impl<Req, Other, Rest...> : std::integral_constant<Idx, 1 + index_impl<Req, Rest...>::value>
		{};

		template<typename T>
		using indexOf = index_impl<T, Args...>;

		// Data
		void* ptr_ = nullptr;
		Idx idx_ = 0;
};