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
//      - Const and Non-Const variants
//      - Test every functionality of the interface
//    3) Code review/re-read.

#pragma once

#include <iterator>

namespace fox {
  namespace detail {
    /// Traits helper class for StableVectorIteratorImpl to reduce
    /// code duplication when implementing both the const and non-const
    /// variants of the iterator.
    template<typename ContainerTy, bool isConst>
    class StableVectorIteratorTraits {
      /* for non-const iterators */
      public:
        using value_type          = typename ContainerTy::value_type;
        using reference           = typename ContainerTy::reference;
        using pointer             = typename ContainerTy::pointer;
        using container           = ContainerTy;
        using container_reference = ContainerTy&;
        using container_pointer   = ContainerTy*;
        using container_iterator  = typename ContainerTy::iterator;
    };

    /// StableVectorIteratorTraits for const iterators
    template<typename ContainerTy>
    class StableVectorIteratorTraits<ContainerTy, true> {
      /* for const iterators */
      public:
        using value_type          = const typename ContainerTy::value_type;
        using reference           = typename ContainerTy::const_reference;
        using pointer             = typename ContainerTy::const_pointer;
        using container           = const ContainerTy;
        using container_reference = const ContainerTy&;
        using container_pointer   = const ContainerTy*;
        using container_iterator  = typename ContainerTy::const_iterator;
    };
  }

  /// StableVectorIteratorImpl is a custom vector iterator with a few additions:
  /// Stability is guaranteed for:
  ///   - The 'end' iterator is guaranteed
  ///   - Inserting/erasing AFTER the iterator's position.
  /// Stability IS NOT guaranteed for
  ///   - Insertion/erasure before the iterator.
  /// Additional notes:
  ///   - The iterator will assert that it's dereferenceable (that it points
  ///     to a valid element
  ///   - ContainerTy (the first template argument) needs to be a std::vector
  ///     or another data structure with the same interface as a std::vector.
  ///     (e.g. llvm::SmallVector)
  /// TODO: This could be a RandomAccessIterator, however I don't need that
  /// functionality for now so I haven't implemented it.
  template<typename ContainerTy, bool isConst>
  class StableVectorIteratorImpl {
    using trait = 
      detail::StableVectorIteratorTraits<ContainerTy, isConst>;
    using this_type = StableVectorIteratorImpl<ContainerTy, isConst>;
    public:
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
      StableVectorIteratorImpl() = default;

      /// Constructor
      /// \param data the container
      /// \param index the index
      explicit StableVectorIteratorImpl(container_reference data, 
                                        std::size_t index = 0) : 
                                        data_(&data), index_(index) {}

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
        // TODO 
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
        // TODO
      }

    private:
      // bool isDereferenceable() const
      // container_reference getContainer() const
      //  check that it's non null (else it's not dereferenceable)
      // void moveToEnd() (sets index_ = npos)
      // void isEnd() 
      // bool compareContainers(const this_type& other) const
      // pointer get() const 
      //    Returns the pointer to the element if dereferenceable. assert that
      //    index < container.size() (else it's not dereferenceable)
      // void advance(difference_type t) (both positive and negative arg)

      static constexpr std::size_t npos = -1;
      container_pointer data_ = nullptr;
      std::size_t index_ = 0;
  };

  template<typename Container>
  using StableVectorIterator = StableVectorIteratorImpl<Container, false>;

  template<typename Container>
  using StableVectorConstIterator = StableVectorIteratorImpl<Container, true>;
}