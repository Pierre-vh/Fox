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

  /**
  * \brief The CustomLinearAllocator class implements a "Pointer-Bump" allocator.
  *
  * This works by allocating pools and giving chunks of it when allocate is called.
  * Allocation is a lot faster, at the cost of less control over memory allocated.
  * Once a block of memory is allocated, you cannot deallocate it 
  * (no Deallocate method).
  *
  * The allocator is pretty similar to LLVM's BumpPtrAllocator.
  *
  * This is useful for allocating lots of long lived object (such as AST Nodes)
  *
  */
  template<
      // Allocate 4096 byte pools by default
      std::size_t poolSize = 4096,
      // Force allocations of objects whose size is >4096 in their own pool.
      std::size_t sizeThresold = poolSize
    >              
  class CustomLinearAllocator {
    public:
      // Typedefs
      using size_type = std::size_t;
      using byte_type = unsigned char;
      using align_type = std::uint8_t;

      /// Returns the pool size
      static constexpr size_t getPoolSize() {
        return poolSize;
      }

      /// Returns the size thresold
      static constexpr size_t getSizeThresold() {
        return sizeThresold;
      }

      // Assertions
      static_assert(poolSize >= 64, "Poolsize cannot be smaller than 64 Bytes");
      static_assert(sizeThresold <= poolSize, "sizeThresold must be <= poolSize");

    private:
      using ThisType = CustomLinearAllocator<poolSize, sizeThresold>;
      
      /// A vector storing the pointers to every pool owned by
      /// this allocator.
      llvm::SmallVector<byte_type*, 4> pools_;
      llvm::SmallVector<std::pair<byte_type*, size_type>, 4> customPools_;

      /// The current allocation pointer.
      byte_type* allocPtr_ = nullptr;

      /// The pointer to the end of the current pool
      byte_type* endPool_ = nullptr;

      /// The pointer to the beginning of the current pool
      byte_type* begPool_ = nullptr;

    public:
      /// The maximum size of a pool
      static constexpr size_type maxPoolSize = 0xFFFFFFFF;

      /// (Default) constructor
      CustomLinearAllocator() = default;

      // Disable copy/move
      CustomLinearAllocator(ThisType&) = delete;
      CustomLinearAllocator(ThisType&&) = delete;
      CustomLinearAllocator& operator=(ThisType&) = delete;

      /// Resets the allocator, freeing allocated pools.
      void reset() {
        // Free "normal" pools
        for (byte_type* pool : pools_)
          std::free(pool);
        // Free "custom" pools
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
      LLVM_ATTRIBUTE_RETURNS_NONNULL 
      LLVM_ATTRIBUTE_RETURNS_NOALIAS 
      void* allocate(size_type size, align_type align = 1) {
        auto paddedSize = size + (align - 1);
        // Allocate in a custom pool if the padded size is bigger
        // than the thresold.
        if (paddedSize > sizeThresold) {
          // Create a custom pool
          byte_type* ptr = createCustomPool(paddedSize);
          // Align its pointer
          ptr = getAlignedPtr(ptr, align);
          assert(ptr && "allocated memory is nullptr!");
          // Return it.
          return ptr;
        }
        // Check if we can allocate in the current pool
        if(allocPtr_) {
          assert(endPool_ && begPool_ && "We have an allocPtr_ but no begPool_ or endPool_?");
          // Allocate in the current pool: check that alignedAllocPtr+size
          // is within bounds.
          byte_type* alignedAllocPtr = getAlignedPtr(allocPtr_, align);
          if ((alignedAllocPtr + size) <= endPool_) {
            // If we can alloc in the current pool, update the allocPtr_ and return
            // alignAllocPtr.
            allocPtr_ = alignedAllocPtr + size;
            assert(alignedAllocPtr && "allocated memory is nullptr!");
            return alignedAllocPtr;
          }
        }
        // Can't allocate in this pool: create a new pool
        createNewPool();
        // Align the allocPtr_
        byte_type* alignedAllocPtr = getAlignedPtr(allocPtr_, align);
        // Calculate the new value of allocPtr
        allocPtr_ = alignedAllocPtr + size;
        // And return the aligned alloc ptr.
        assert(alignedAllocPtr && "allocated memory is nullptr!");
        return alignedAllocPtr;
      }

      
      /// Templated version of allocate
      template<typename DataTy>
      LLVM_ATTRIBUTE_RETURNS_NONNULL 
      LLVM_ATTRIBUTE_RETURNS_NOALIAS 
      DataTy* allocate() {
        return static_cast<DataTy*>(allocate(sizeof(DataTy), alignof(DataTy)));
      }

      
      /// Deallocates the pointer. This just zeroes the memory 
      /// unless it's the last object allocated, then, it's actually freed.
      /// \param ptr The pointer which holds the chunk of memory you own.
      /// \param sz The size of the chunk of memory in bytes.
      void deallocate(const void*, size_type) {
        // No-op, since we don't deallocate memory in this allocator, it's
        // only freed when destroyAll is called.
      }

      /// Templated version of deallocate()
      template<typename DataTy>
      void deallocate(const DataTy* ptr) {
        deallocate(ptr, sizeof(DataTy));
      }

      ~CustomLinearAllocator() {
        reset();
      }

      /// Prints a dump of the allocator to std::cerr
      void dump() const {
      #pragma message("fix total memory allocated count!")
        detail::doLinearAllocatorDump(getTotalPoolsCount(),
                                      getBytesInCurrentPool(), 0);
      }

      /// Returns the total number of pools created.
      size_type getTotalPoolsCount() const {
        return pools_.size() + customPools_.size();
      }

      /// Returns the number of "normal" pools created
      size_type getNormalPoolsCount() const {
        return pools_.size();
      }

      /// Returns the number of "custom" pools
      size_type getCustomPoolsCount() const {
        return customPools_.size();
      }

      /// Returns the number of bytes allocated in the current pool.
      size_type getBytesInCurrentPool() const {
        if (!allocPtr_) return 0;
        assert(begPool_ && "allocPtr_ is not null, but begPool_ is?");
        return static_cast<size_type>(allocPtr_ - begPool_);
      }

    private:
      /// Returns ptr, aligned to align.
      byte_type* getAlignedPtr(byte_type* ptr, align_type align) {
        // Alignement related checks
        assert((align > 0) 
          && "Alignement cannot be less or equal to zero");
        assert(((align == 1) || ((align & (align - 1)) == 0))
          && "Alignement must be 1, or a power of 2");

        // Don't bother calculating anything if no alignement
        // is required
        if (align == 1) return ptr;

        // Calculate the aligned ptr
        auto ptrInt = reinterpret_cast<std::uintptr_t>(ptr);
        ptrInt += align - (ptrInt % align);
        return reinterpret_cast<byte_type*>(ptrInt);
      }

      /// Calculates the size that a new pool should have
      /// The size of the pool doubles every 128 allocations, and
      /// maxes out at 4Gb (1 << 32)
      size_type calculateNewPoolSize() const {
        size_type factor = std::max<size_type>(pools_.size() / 128, 1);
        assert(factor && "factor is zero!");
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

      /// Creates a new "normal" pool, and sets allocPtr_ and endPool_.
      void createNewPool() {
        size_type size = calculateNewPoolSize();
        // Allocate the new pool
        begPool_ = (byte_type*)llvm::safe_malloc(size);
        endPool_ = begPool_ + size;
        allocPtr_ = begPool_;
        // Register the pool
        pools_.push_back(begPool_);
        assert(begPool_ && allocPtr_ && endPool_ && pools_.size());
      }
  };

  using LinearAllocator = CustomLinearAllocator<>;
}