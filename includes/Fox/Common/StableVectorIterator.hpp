//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : StableVector.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the "StableVectorIterator" class
//----------------------------------------------------------------------------//

#pragma once

#include <iterator>
#include <limits>
#include "Errors.hpp"

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
        using container_iterator  = typename ContainerTy::const_iterator;
    };
  }

  /// StableVectorIteratorImpl is a custom index-based 'stable' vector iterator
  /// Stability is guaranteed for:
  ///   - The 'end' iterator is guaranteed
  ///   - Inserting/erasing AFTER the iterator's position.
  /// Stability IS NOT guaranteed for
  ///   - Insertion/erasure before or at the iterator
  /// Additional notes:
  ///   - As this is index based, the result of the operator* is not stable.
  ///     Don't store something like 'auto& elem = (*iter);' just like you
  ///     wouldn't store a vector iterator.
  ///   - The iterator will assert that it's dereferenceable (that it points
  ///     to a valid element
  ///   - ContainerTy (the first template argument) needs to be a std::vector
  ///     or another data structure with the same interface as a std::vector.
  ///     (e.g. llvm::SmallVector)
  template<typename ContainerTy, bool isConst>
  class StableVectorIteratorImpl {
    using trait = 
      detail::StableVectorIteratorTraits<ContainerTy, isConst>;
    using this_type = StableVectorIteratorImpl<ContainerTy, isConst>;
    using const_iter = StableVectorIteratorImpl<ContainerTy, true>;
    public:
      using iterator_category   = std::bidirectional_iterator_tag;
      using difference_type     = typename ContainerTy::difference_type;
      using value_type          = typename trait::value_type;
      using reference           = typename trait::reference;
      using pointer             = typename trait::pointer;
      using container           = typename trait::container;
      using container_iterator  = typename trait::container_iterator;

      /// Default Constructor
      StableVectorIteratorImpl() = default;

      /// Creates a StableVectorIterator from a container and an index.
      /// NOTE: for empty vectors, this will always create a
      /// begin iterator if you pass '0'.
      /// If you want a end() iterator for a maybe empty container, use
      /// ::getEnd()
      explicit StableVectorIteratorImpl(container& data, std::size_t idx = 0) :
        data_(&data) {
        assert(idx <= data.size());
        // if index equals the size of the container, and the container is
        // not empty, create a end iterator.
        if((idx == data.size()) && data.size())
          index_ = endpos;
        else 
          index_ = idx;
      }

      /// Creates an iterator to past-the-end of \p data
      static this_type getEnd(container& data) {
        return this_type(data, endpos);
      }

      /// Creates an iterator to the beginning of \p data
      static this_type getBegin(container& data) {
        return this_type(data);
      }

      /// Creates a StableVectorIterator from a container and an iterator.
      /// NOTE: for empty vectors (.begin() == .end()), this will create a
      /// begin iterator even if you pass a .end() iterator.
      /// If you want a end() iterator for a maybe empty container, use
      /// ::getEnd()
      StableVectorIteratorImpl(container& data, container_iterator iter) : 
         StableVectorIteratorImpl(data, std::distance(data.begin(), iter)) {}

      // Allow non-const iterators to implicitely cast to const iterators.
      operator const_iter() {
        if(this->data_)
          return const_iter(*this->data_, this->index_);
        assert((this->index_ == 0) 
          && "Iterator does not have data, but it has a non-zero index?");
        return const_iter();
      }

      // Pre-increment
      this_type& operator++() {
        advance(1);
        return (*this);
      }

      // Post-increment
      this_type operator++(int) {
        auto save = (*this);
        ++(*this);
        return save;
      }

      // Pre-decrement
      this_type& operator--() {
        advance(-1);
        return (*this);
      }

      // Post-decrement
      this_type operator--(int) {
        auto save = (*this);
        --(*this);
        return save;
      }

      reference operator*() const {
        return *get();
      }

      pointer operator->() const {
        return get();
      }

      /// Converts this iterator to a 'vanilla' container iterator.
      /// (ContainerTy::iterator or ContainerTy::const_iterator, depending
      /// on if this iterator is constant or not)
      container_iterator getContainerIterator() const {
        if(isEnd())
          return getContainer().end();
        return getContainer().begin()+index_;
      }

      friend bool operator==(const this_type& lhs, const this_type& rhs) {
        return lhs.getContainerIterator() == rhs.getContainerIterator();
      }

      friend bool operator!=(const this_type& lhs, const this_type& rhs) {
        return lhs.getContainerIterator() != rhs.getContainerIterator();
      }

      friend bool operator<(const this_type& lhs, const this_type& rhs) {
        return lhs.getContainerIterator() < rhs.getContainerIterator();
      }

      friend bool operator<=(const this_type& lhs, const this_type& rhs) {
        return lhs.getContainerIterator() <= rhs.getContainerIterator();
      }
      
      friend bool operator>(const this_type& lhs, const this_type& rhs) {
        return lhs.getContainerIterator() > rhs.getContainerIterator();
      }

      friend bool operator>=(const this_type& lhs, const this_type& rhs) {
        return lhs.getContainerIterator() >= rhs.getContainerIterator();
      }

      this_type& operator+=(difference_type value) {
        advance(value);
        return *this;
      }

      this_type& operator-=(difference_type value) {
        advance(-value);
        return *this;
      }

      friend this_type operator+(this_type iter, difference_type value) {
        iter += value;
        return iter;
      }

      friend this_type operator-(this_type iter, difference_type value) {
        iter -= value;
        return iter;
      }

      friend this_type operator+(difference_type value, this_type iter) {
        iter += value;
        return iter;
      }

      friend this_type operator-(difference_type value, this_type iter) {
        iter -= value;
        return iter;
      }

      /// Calculates the distance between 2 iterators, \p first and \p last.
      friend difference_type
      distance(const this_type& first, const this_type& last) {
        assert(first.canCompareWith(last) && "iterators aren't comparable");
        return std::distance(first.getContainerIterator(),
                             last.getContainerIterator());
      }
    protected:
      /// Sentinel value for end iterators
      static constexpr std::size_t
        endpos = std::numeric_limits<std::size_t>::max();
      /// The container
      container* data_ = nullptr;
      /// The index in the container
      std::size_t index_ = 0;

    private:
      
      /// \returns true if this iterator is dereferenceable.
      bool isDereferenceable() const {
        // Check that we have a container
        if(!data_) return false;
        // Check that we arent an end iterator
        if(index_ == endpos) return false;
        // Check that our element is accessible and isn't end.
        return (data_->size() > index_);
      }

      /// Returns the container, checking that we have one in the process.
      container& getContainer() const {
        assert(data_ && "Iterator is not dereferenceable, "
          "it does not have a target container");
        return (*data_);
      }

      /// Checks if this is an end iterator
      bool isEnd() const {
        return (index_ == endpos);
      }

      /// Checks if this iterator shares the same container as \p other
      bool compareContainers(const this_type& other) const {
        return (data_ == other.data_);
      }

      /// \returns true if both this and \p other have the same non-null
      /// container
      bool canCompareWith(const this_type& other) const {
        if(compareContainers(other))
          return (data_ != nullptr);
        return false;
      }

      /// Returns
      pointer get() const {
        assert(isDereferenceable() 
          && "Iterator is not dereferenceable");
        return &(*getContainerIterator());
      }

      /// Moves the iterator. Can have a negative argument to
      /// decrement the iterator.
      void advance(difference_type value) {
        assert(data_ 
          && "cannot move iterator: iterator does not have a container");
        if(value == 0) return;
        // Special case, check if we want to move past-the-end
        if ((value > 0) && (value == endpos)) {
          index_ = endpos;
          return;
        }
        assert(((index_ < getContainer().size()) || (index_ == endpos))
          && "Element pointed by the iterator does not exist anymore");
        // value < 0 : decrement
        if (value < 0) {
          // Note: keep in mind that the value is negative in this branch,
          // so when we add the value we're actually substracting.
          assert((index_ > 0) 
            && "Decrementing a begin iterator");
          // Decrementing the past-the-end iterator.
          if (isEnd())
            index_ = (getContainer().size() + value);
          // Decrementing any other iterator
          else {
            assert(((std::size_t)(-value) <= index_) 
              && "Decrementing the iterator too much: "
                 "result would be before the begin iterator");
            index_ += value;
          }
        }
        // value > 0 : increment
        else {
          assert(!isEnd() 
            && "Incrementing a past-the-end iterator");
          assert(((index_ + value) <= getContainer().size())
            && "Incrementing the iterator too much: "
               "result would be after the end iterator");
          index_ += value;
          // if we reached the end, make the index_ endpos.
          if(index_ == getContainer().size()) 
            index_ = endpos;
        }
      }
  };

  /// The 'StableVectorIterator'.
  /// See \ref StableVectorIteratorImpl for more information
  template<typename Container>
  using StableVectorConstIterator = StableVectorIteratorImpl<Container, true>;

  /// The 'StableVectorIterator'.
  /// See \ref StableVectorIteratorImpl for more information
  template<typename Container>
  using StableVectorIterator = StableVectorIteratorImpl<Container, false>;
}