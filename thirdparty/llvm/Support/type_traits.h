//===- llvm/Support/type_traits.h - Simplfied type traits -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides useful additions to the standard type_traits library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_TYPE_TRAITS_H
#define LLVM_SUPPORT_TYPE_TRAITS_H

#include <type_traits>
#include <utility>

namespace llvm {

	/// isPodLike - This is a type trait that is used to determine whether a given
	/// type can be copied around with memcpy instead of running ctors etc.
	template <typename T>
	struct isPodLike {
		static const bool value = std::is_trivially_copyable<T>::value;
	};

	// std::pair's are pod-like if their elements are.
	template<typename T, typename U>
	struct isPodLike<std::pair<T, U>> {
		static const bool value = isPodLike<T>::value && isPodLike<U>::value;
	};

	/// Metafunction that determines whether the given type is either an
	/// integral type or an enumeration type, including enum classes.
	///
	/// Note that this accepts potentially more integral types than is_integral
	/// because it is based on being implicitly convertible to an integral type.
	/// Also note that enum classes aren't implicitly convertible to integral types,
	/// the value may therefore need to be explicitly converted before being used.
	template <typename T> class is_integral_or_enum {
		using UnderlyingT = typename std::remove_reference<T>::type;

	public:
		static const bool value =
			!std::is_class<UnderlyingT>::value && // Filter conversion operators.
			!std::is_pointer<UnderlyingT>::value &&
			!std::is_floating_point<UnderlyingT>::value &&
			(std::is_enum<UnderlyingT>::value ||
				std::is_convertible<UnderlyingT, unsigned long long>::value);
	};

	/// If T is a pointer, just return it. If it is not, return T&.
	template<typename T, typename Enable = void>
	struct add_lvalue_reference_if_not_pointer { using type = T & ; };

	template <typename T>
	struct add_lvalue_reference_if_not_pointer<
		T, typename std::enable_if<std::is_pointer<T>::value>::type> {
		using type = T;
	};

	/// If T is a pointer to X, return a pointer to const X. If it is not,
	/// return const T.
	template<typename T, typename Enable = void>
	struct add_const_past_pointer { using type = const T; };

	template <typename T>
	struct add_const_past_pointer<
		T, typename std::enable_if<std::is_pointer<T>::value>::type> {
		using type = const typename std::remove_pointer<T>::type *;
	};

	template <typename T, typename Enable = void>
	struct const_pointer_or_const_ref {
		using type = const T &;
	};
	template <typename T>
	struct const_pointer_or_const_ref<
		T, typename std::enable_if<std::is_pointer<T>::value>::type> {
		using type = typename add_const_past_pointer<T>::type;
	};

	namespace detail {
		/// Internal utility to detect trivial copy construction.
		template<typename T> union copy_construction_triviality_helper {
			T t;
			copy_construction_triviality_helper() = default;
			copy_construction_triviality_helper(const copy_construction_triviality_helper&) = default;
			~copy_construction_triviality_helper() = default;
		};
		/// Internal utility to detect trivial move construction.
		template<typename T> union move_construction_triviality_helper {
			T t;
			move_construction_triviality_helper() = default;
			move_construction_triviality_helper(move_construction_triviality_helper&&) = default;
			~move_construction_triviality_helper() = default;
		};
	} // end namespace detail

	  /// An implementation of `std::is_trivially_copy_constructible` since we have
	  /// users with STLs that don't yet include it.
	template <typename T>
	struct is_trivially_copy_constructible
		: std::is_copy_constructible<
		::llvm::detail::copy_construction_triviality_helper<T>> {};
	template <typename T>
	struct is_trivially_copy_constructible<T &> : std::true_type {};
	template <typename T>
	struct is_trivially_copy_constructible<T &&> : std::false_type {};

	/// An implementation of `std::is_trivially_move_constructible` since we have
	/// users with STLs that don't yet include it.
	template <typename T>
	struct is_trivially_move_constructible
		: std::is_move_constructible<
		::llvm::detail::move_construction_triviality_helper<T>> {};
	template <typename T>
	struct is_trivially_move_constructible<T &> : std::true_type {};
	template <typename T>
	struct is_trivially_move_constructible<T &&> : std::true_type {};

} // end namespace llvm

#endif // LLVM_SUPPORT_TYPE_TRAITS_H