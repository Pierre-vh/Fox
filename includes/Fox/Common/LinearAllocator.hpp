//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : LinearAllocator.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Implements a custom Linear Allocator.
// This allocator does not inherit from std::allocator_traits
//----------------------------------------------------------------------------//

#pragma once

#include "llvm/Support/MemAlloc.h"
#include "llvm/Support/Compiler.h"
#include "llvm/ADT/SmallVector.h"
#include "Errors.hpp"
#include <tuple>

namespace fox {
  namespace detail {
    void doLinearAllocatorDump(std::size_t numPools, 
      std::size_t bytesInCurrentPool, std::size_t totalBytesUsed);
  }

  
  /// \brief The CustomLinearAllocator class implements a "Pointer-Bump" 
  ///        allocator.
  ///
  /// This works by allocating pools and giving chunks of it when allocate
  /// is called.
  /// Allocation is a lot faster, at the cost of less control over memory
  /// allocated.
  /// Once a block of memory is allocated, you cannot deallocate it 
  /// (no "real" Deallocate method).
  ///
  /// This implementation is pretty similar to LLVM's BumpPtrAllocator.
  template<
      // Allocate 4096 bytes pools by default
      std::size_t poolSize = 4096,
      // Force allocations of objects whose size is >4096 in their own pool.
      std::size_t sizeThresold = poolSize
    >              
  class CustomLinearAllocator {
    public:
      /// The type used to represent sizes
      using size_type = std::size_t;

      /// The type used to represent bytes
      using byte_type = unsigned char;

      /// The type used to represent alignement
      using align_type = std::uint8_t;

      /// \return the pool size
      static constexpr size_t getPoolSize() {
        return poolSize;
      }

      /// \return the size thresold
      static constexpr size_t getSizeThresold() {
        return sizeThresold;
      }

      static_assert(poolSize >= 64, 
        "Poolsize cannot be smaller than 64 Bytes");
      static_assert(sizeThresold <= poolSize, 
        "sizeThresold must be <= poolSize");

    private:
      using ThisType = CustomLinearAllocator<poolSize, sizeThresold>;
      
      /// The "normal" pools created by the allocator
      llvm::SmallVector<byte_type*, 4> pools_;

      /// The "custom" pools created by the allocator
      llvm::SmallVector<std::pair<byte_type*, size_type>, 4> customPools_;

      /// The current allocation pointer.
      byte_type* allocPtr_ = nullptr;

      /// The pointer to the end of the current pool
      byte_type* endPool_ = nullptr;

      /// The pointer to the beginning of the current pool
      byte_type* begPool_ = nullptr;

      /// Make the allocator non-copyable and non-movable. This is 
      /// needed to avoid allocators sharing pools (that'd be a
      /// disaster!)
      CustomLinearAllocator(ThisType&) = delete;
      CustomLinearAllocator(ThisType&&) = delete;
      CustomLinearAllocator& operator=(ThisType&) = delete;
      CustomLinearAllocator& operator=(ThisType&&) = delete;

    public:
      /// The maximum size of a pool. Currently, 4GB.
      static constexpr size_type maxPoolSize = 0xFFFFFFFF;

      CustomLinearAllocator() = default;

      /// Resets the allocator, freeing any allocated memory.
      void reset() {
        // Free the normal pools
        for (byte_type* pool : pools_)
          std::free(pool);
        // Free the custom pools
        for (auto pair: customPools_)
          std::free(pair.first);
        // Clear the pool vectors
        pools_.clear();
        customPools_.clear();
        // Reset the alloc pointers
        allocPtr_ = nullptr;
        endPool_ = nullptr;
        begPool_ = nullptr;
      }

      /// Allocates "size" bytes of memory with "align" alignement.
      /// \return the allocated memory block, which should never
      ///         be nullptr unless we ran out of memory.
      LLVM_ATTRIBUTE_RETURNS_NONNULL 
      LLVM_ATTRIBUTE_RETURNS_NOALIAS 
      void* allocate(size_type size, align_type align = 1) {
        // The size of the allocation + some spare size for padding that might
        // be needed.
        auto paddedSize = size + (align - 1);
        // Allocate in a custom pool if the padded size is bigger
        // than the thresold.
        if (paddedSize > sizeThresold) {
          byte_type* ptr = createCustomPool(paddedSize);
          ptr = getAlignedPtr(ptr, align);
          assert(ptr && "allocated memory is nullptr!");
          return ptr;
        }
        // Allocate in the current pool when possible.
        if(allocPtr_) {
          assert(endPool_ && begPool_ 
            && "We have an allocPtr_ but no begPool_ or endPool_?");
          // Allocate in the current pool: check that alignedAllocPtr+size
          // is within bounds.
          byte_type* alignedAllocPtr = getAlignedPtr(allocPtr_, align);
          if ((alignedAllocPtr + size) <= endPool_) {
            // If we can alloc in the current pool, update allocPtr_ and return
            // alignAllocPtr.
            allocPtr_ = alignedAllocPtr + size;
            assert(alignedAllocPtr && "allocated memory is nullptr!");
            return alignedAllocPtr;
          }
        }
        // Can't allocate in this pool, but a normal pool can 
        // support this allocation: create a new pool.
        createNewPool();
        byte_type* alignedAllocPtr = getAlignedPtr(allocPtr_, align);
        allocPtr_ = alignedAllocPtr + size;
        assert(alignedAllocPtr && "allocated memory is nullptr!");
        return alignedAllocPtr;
      }

      
      /// Templated version of allocate, which uses sizeof() and alignof()
      /// to call the base allocate method.
      template<typename DataTy>
      LLVM_ATTRIBUTE_RETURNS_NONNULL 
      LLVM_ATTRIBUTE_RETURNS_NOALIAS 
      DataTy* allocate() {
        return static_cast<DataTy*>(
          allocate(sizeof(DataTy), alignof(DataTy))
        );
      }

      /// no-op, since this allocator doesn't free memory until reset() 
      /// is called.
      void deallocate(const void*, size_type) {}

      /// no-op, since this allocator doesn't free memory until reset() 
      /// is called.
      template<typename DataTy>
      void deallocate(const DataTy* ptr) {
        //deallocate(ptr, sizeof(DataTy));
      }

      /// Destructor that calls reset()
      ~CustomLinearAllocator() {
        reset();
      }

      /// Prints a dump of the allocator to std::cerr
      void dump() const {
        detail::doLinearAllocatorDump(getTotalPoolsCount(),
                                      getBytesInCurrentPool(), 
                                      getTotalBytesAllocated());
      }

      /// \return the total number of pools created
      size_type getTotalPoolsCount() const {
        return pools_.size() + customPools_.size();
      }

      /// \return the number of "normal" pools created
      size_type getNormalPoolsCount() const {
        return pools_.size();
      }

      /// \return the number of "custom" pools created
      size_type getCustomPoolsCount() const {
        return customPools_.size();
      }

      /// \return the number of bytes allocated in the current (normal) pool
      size_type getBytesInCurrentPool() const {
        if (!allocPtr_) return 0;
        assert(begPool_ && "allocPtr_ is not null, but begPool_ is?");
        return static_cast<size_type>(allocPtr_ - begPool_);
      }

      /// \return the total number of bytes allocated.
      size_type getTotalBytesAllocated() const {
        size_t total = 0;
        // iterate over every pool except the last one
        for (size_t idx = 0, sz = pools_.size(); (idx+1) < sz; ++idx) {
          total += calculatePoolSize(idx);
        }
        // Add the number of bytes in the current pool
        total += getBytesInCurrentPool();
        // Add the bytes allocated in custom pools
        for (auto pair : customPools_) {
          total += pair.second;
        }
        return total;
      }

    private:
      /// \param ptr the pointer to align
      /// \param align the alignement
      /// \return ptr, correctly aligned.
      byte_type* getAlignedPtr(byte_type* ptr, align_type align) {
        assert((align > 0) 
          && "Alignement cannot be less or equal to zero");
        assert(((align == 1) || ((align & (align - 1)) == 0))
          && "Alignement must be 1, or a power of 2");

        // Don't bother calculating anything if no alignement
        // is required
        if (align == 1) return ptr;

        // Calculate the aligned ptr.
        return ptr + (align - (reinterpret_cast<std::uintptr_t>(ptr) % align));
      }

      /// Calculates the size of a pool at index idx.
      /// The size of the pool doubles every 128 allocations, and
      /// maxes out at maxPoolSize's value.
      ///
      /// \return the size of the pool
      size_type calculatePoolSize(size_type idx) const {
        if(idx < 128) return poolSize;
        size_type factor = std::max<size_type>(idx / 128, 1);
        assert((factor > 0) && "factor is zero!");
        // Return either the size, or the maximum pool size, depending
        // on whichever is smaller.
        size_type size = std::min(factor*poolSize, maxPoolSize);
        assert(size >= poolSize && "size is too small!");
        return size;
      }

      /// Creates a new custom pool
      /// \param poolSize The size of the pool to allocate
      /// \return a pointer to the beginning of the pool
      byte_type* createCustomPool(size_type size) {
        assert(poolSize >= sizeThresold && "doesn't deserve its own pool!");
        // Allocate the pool
        byte_type* ptr = (byte_type*)llvm::safe_malloc(size);
        // Register  it
        customPools_.push_back({ptr, size});
        // Return it
        assert(ptr && customPools_.size());
        return ptr;
      }

      /// Creates a new "normal" pool and sets allocPtr_, begPool_ and endPool_
      void createNewPool() {
        size_type size = calculatePoolSize(pools_.size());
        // Allocate the new pool
        begPool_ = (byte_type*)llvm::safe_malloc(size);
        endPool_ = begPool_ + size;
        allocPtr_ = begPool_;
        // Register the pool
        pools_.push_back(begPool_);
        assert(begPool_ && allocPtr_ && endPool_ && pools_.size());
      }
  };

  /// The basic linear allocator, which uses the default template parameter.
  using LinearAllocator = CustomLinearAllocator<>;
}