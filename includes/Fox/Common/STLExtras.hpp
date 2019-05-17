//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : STLExtras.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains a few extras added to the STL. Usually it's used for
// functions that aren't available in the current C++ version.
//----------------------------------------------------------------------------//

#pragma once

#include <tuple>

namespace fox {
  namespace detail {
    template <class F, class Tuple, std::size_t... I>
    constexpr decltype(auto) 
    apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>) {
      return std::invoke(std::forward<F>(f), 
        std::get<I>(std::forward<Tuple>(t))...);
    }
  }  // namespace detail
 
  template <class F, class Tuple>
  constexpr decltype(auto) apply(F&& f, Tuple&& t) {
    return detail::apply_impl(
      std::forward<F>(f), 
      std::forward<Tuple>(t),
      std::make_index_sequence<
        std::tuple_size_v<std::remove_reference_t<Tuple>>>{}
    );
  }
}