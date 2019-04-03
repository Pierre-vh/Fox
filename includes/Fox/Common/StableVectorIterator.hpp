//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : StableVector.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the "StableVectorIterator" class
//----------------------------------------------------------------------------//

// TODO: 
//    1) Finish the implementation
//    2) Test this thouroughly
//    3) Code review

#pragma once

#include <iterator>

namespace fox {
  namespace detail {
    /// Traits helper class for StableVectorIteratorImpl to reduce
    /// code duplication when implementing both the const and non-const
    /// variants of the iterator.
    template<typename ContainerTy, typename ElemTy, bool isConst>
    class StableVectorIteratorTraits {
      /* for non-const iterators */
      public:
        using value_type          = ElemTy;
        using reference           = ElemTy&;
        using pointer             = ElemTy*;
        using container           = ContainerTy;
        using container_reference = ContainerTy&;
        using container_pointer   = ContainerTy*;
        using container_iterator  = typename ContainerTy::iterator;
    };

    /// StableVectorIteratorTraits for const iterators
    template<typename ContainerTy, typename ElemTy>
    class StableVectorIteratorTraits<ContainerTy, ElemTy, true> {
      /* for const iterators */
      public:
        using value_type          = const ElemTy;
        using reference           = const ElemTy&;
        using pointer             = const ElemTy*;
        using container           = const ContainerTy;
        using container_reference = const ContainerTy&;
        using container_pointer   = const ContainerTy*;
        using container_iterator  = typename ContainerTy::const_iterator;
    };
  }

  /// StableVectorIteratorImpl is a vector iterator that guarantees stability
  /// in some scenarios.
  ///   - Stability of the 'end' iterator is guaranteed
  ///   - Stability of iterators is guaranteed when inserting/erasing AFTER the 
  ///     iterator's position.
  /// Stability IS NOT guaranteed for insertion/erasure before the iterator.
  template<typename ContainerTy, typename ElemTy, bool isConst>
  class StableVectorIteratorImpl {
    using trait = 
      detail::StableVectorIteratorTraits<ContainerTy, ElemTy, isConst>;
    using this_type = StableVectorIteratorImpl<ContainerTy, ElemTy, isConst>;
    public:
      using index_type          = std::size_t;
      using iterator_category   = std::bidirectional_iterator_tag;
      using difference_type     = std::ptrdiff_t;
      using value_type          = typename trait::value_type;
      using reference           = typename trait::reference;
      using pointer             = typename trait::pointer;
      using container           = typename trait::container;
      using container_reference = typename trait::container_reference;
      using container_pointer   = typename trait::container_pointer;
      using container_iterator  = typename trait::container_iterator;

      /// Default Constructor
      StableVectorIteratorImpl() {
        // TODO
      }

      /// Constructor
      StableVectorIteratorImpl(container_reference container, 
                               index_type idx) {
        // TODO
      }

      // Pre-increment
      this_type& operator++() {
        // TODO
      }

      // Post-increment
      this_type operator++(int) {
        // TODO
      }

      // Pre-decrement
      this_type& operator--() {
        // TODO
      }

      // Post-decrement
      this_type operator--(int) {
        // TODO
      }

      reference operator*() const {
        // TODO
      }

      pointer operator->() const {
        // TODO
      }

      /// Converts this iterator to a 'vanilla' container iterator.
      /// (ContainerTy::iterator or ContainerTy::const_iterator, depending
      /// on if this iterator is constant or not)
      container_iterator getContainerIterator() const {
        
      }

      friend bool operator==(const this_type& lhs, const this_type& rhs) {
        // TODO
      }

      friend bool operator!=(const this_type& lhs, const this_type& rhs) {
        // TODO
      }

      /// Calculates the distance between 2 iterators, \p first and \p last.
      friend difference_type
      distance(const this_type& first, const this_type& last) {

      }

    private:
      // bool isDereferenceable() const
      static constexpr index_type npos = -1;
      container_pointer container_ = nullptr;
      index_type index_ = 0;
  };
}