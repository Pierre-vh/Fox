////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : LinearAllocator.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Implements a custom Linear Allocator.
// This allocator does not inherit from std::allocator_traits
////------------------------------------------------------////

// LinearAllocator To-Do list:
	// Make it alignement-aware (Done? Sort of.. Needs to be checked/tested)
	// Allow "array" allocations

#pragma once

#include <cstddef>
#include <memory>
#include <iostream>
#include "Errors.hpp"

namespace fox
{
	// Helper for units
	template<std::uint16_t size>
	class KiloByte
	{
		public:
			static constexpr std::uint32_t value = size * 1000;
	};

	template<std::uint16_t size>
	class MegaByte
	{
		public:
			static constexpr std::uint32_t value = KiloByte<size>::value * 1000;
	};

	/*
		\brief The LinearAllocator class implements a "Pointer-Bump" allocator.
		This works by allocating pools and giving chunks of it when allocate is called.
		Allocation is REALLY fast, at the cost of a less control over memory allocated.
		Once a block of memory is allocated, you cannot deallocate it (no Deallocate method).
		This is useful for allocating lots of long lived object (such as AST Nodes)

		Note: This class does not strive to be thread-safe. Be careful with that!
		Note: This class will assert a lot to guarantee that the memory returned is never null
		when it still has available pools. It will call "reportBadAlloc" if it exceeds the number of
		pools limit

		Set maxPools to 0 for infinite pools.
	*/
	template<
			std::uint32_t poolSize = KiloByte<128>::value, // Allocate 128Kb pools by default
			std::uint16_t maxPools = 0,		// Allow unlimited pools by default
			std::uint8_t poolAlign = 4		// Don't align pools by default
		>							
		class LinearAllocator
		{
			public:
				// Typedefs
				using size_type = std::size_t;
				using byte_type = unsigned char;
				using align_type = std::uint8_t;

				// Assertions
				static_assert(poolSize >= KiloByte<1>::value, "Poolsize cannot be smaller than 1kb");
				static_assert(poolAlign > 0, "Pool alignement must be greater than 0");

			private:
				/*
					\brief A Single Pool.
				*/
				struct alignas(poolAlign) Pool
				{
					Pool(Pool* last) : upperBound(data + (poolSize - 1)), previous(last)
					{
						memset(data, 0, sizeof(data));
					}

					byte_type data[poolSize];
					const byte_type* const upperBound = nullptr;
					std::unique_ptr<Pool> next = nullptr;
					Pool* previous = nullptr;
				};

				std::unique_ptr<Pool> firstPool = nullptr;
				Pool* curPool = nullptr;
				size_type poolCount = 0;
				void* allocPtr = nullptr;

			public:

				/*
					\brief Constructor. Does some checks and calls setup.
				*/
				LinearAllocator()
				{
					setup();
				}

				/*
					\brief	Setup the Allocator for use (creates the first pool)
				*/
				void setup()
				{
					if (!firstPool)
					{
						bool result = createPool();
						assert(result && "Couldn't allocate first pool!");
					}
				}

				/*
					\brief Destroys every pool then creates a new pool. This "resets" the allocator.
				*/
				void reset()
				{
					destroyAll();
					setup();
				}

				/*
					\brief Allocates (size) bytes of memory, bumping the pointer by (size) bytes + eventual padding for alignement.
					\param size The size of the chunk of memory you want to allocate in bytes.
					\returns Your chunk of memory, nullptr if the allocator can't allocate any more memory.
				*/
				void* allocate(size_type size, align_type align = 1)
				{
					assert((align > 0) && "Alignement must be 1 or more!");
					assert(((align == 1) || ((align & (align - 1)) == 0))
						&& "Alignement must equal to 1, or a power of 2.");
					assert(allocPtr && "AllocPtr cannot be null");

					// Check if the object fits
					if (!doesObjectFit(size, align))
						reportBadAlloc("Object too big");

					// See if we need to create a new pool
					if (createNewPoolIfRequired(size, align) < 0)
						reportBadAlloc("Maximum number of pools exceeded");


					auto tmp = alignPtr(allocPtr, align);
					allocPtr = static_cast<byte_type*>(tmp) + size;
					
					if (!tmp)
						reportBadAlloc("Pointer returned is null");

					return tmp;
				}

				/*
					\brief Templated version of allocate which uses sizeof() to call the base allocate. See non-templated allocate overload for more details.
				*/
				template<typename DataTy>
				auto allocate()
				{
					static_assert(doesObjectFit(sizeof(DataTy), alignof(DataTy)), 
						"Object too big for allocator");
					return static_cast<DataTy*>(allocate(sizeof(DataTy), alignof(DataTy)));
				}

				/*
					\brief Destroys every pool.
				*/
				void destroyAll()
				{
					// Note: Due to the nature of the unique_ptrs, this "destroy" function is useless when called from the destructor, however, it's provided
					// as a mean of resetting the pool.

					// Free the first pool, starting a chain reaction where every pool will be deleted.
					if (firstPool)
						firstPool.reset();

					// Set all the member variables to safe values.
					curPool = nullptr;
					poolCount = 0;
					allocPtr = nullptr;
				}

				/*
					\brief Destructor (just calls reset())
				*/
				~LinearAllocator()
				{
					destroyAll();
				}

				/*
					\returns True if the object will fit in a pool, false otherwise.
				*/
				static constexpr bool doesObjectFit(size_type sz, align_type align)
				{
					// We compare to size + (align-1), to make room for padding if needed
					return (poolSize >= (sz + (align - 1)));
				}

				/*
					\returns True if we can allocate more pools
				*/
				bool canCreateMorePools() const
				{
					// Return true if we have infinite pools or if we still have room for more.
					return (maxPools == 0) || (poolCount < maxPools);
				}

				/*
					\brief Creates a pool. Will create the first one if that is not done yet, else, it'll just add another one at the end of the linked list of pools.
				*/
				bool createPool()
				{
					if (hasReachedMaxPools())
						return false;

					// Curpool isn't -> no pool yet
					if (curPool)
					{
						assert(firstPool && "curPool isn't null, but firstPool is ?");
						curPool->next = std::make_unique<Pool>(curPool);
						curPool = curPool->next.get();
					}
					else
					{
						assert(!firstPool && "curPool is null, but firstPool isn't?");
						firstPool = std::make_unique<Pool>(curPool);
						curPool = firstPool.get();
					}

					poolCount++;
					allocPtr = curPool->data;
					assert(allocPtr && "allocPtr cannot be null");
					return true;
				}

				/*
					\brief	Displays a detailled dump to get an overview of Allocator.
					This really just displays MaxPools and PoolSize and calls smallDump()
				*/
				void dump(std::ostream& os = std::cout) const
				{
					os << "MaxPools: " << maxPools << "\n";
					os << "Pool Size: " << poolSize << "\n";
					smallDump(os);
				}

				/*
					\brief Displays a condensed dump to get an overview of the state of the allocator.
				*/
				void smallDump(std::ostream& os = std::cout) const
				{
					os << "Pools: " << poolCount << "\n";
					os << "Curpool address: " << (void*)curPool << "\n";
					os << "AllocPtr address: " << (void*)allocPtr << "\n";
					os << "Bytes in current pool: " << (std::ptrdiff_t)(((byte_type*)allocPtr) - ((byte_type*)curPool)) << "\n";
				}

				/*
					\brief Returns true if we have exceeded the max number of pools allowed
				*/
				bool hasReachedMaxPools() const
				{
					return !canCreateMorePools();
				}

				size_type getPoolCount() const
				{
					return poolCount;
				}
			private:
				/*
					\brief Aligns a pointer
					\returns the aligned pointer
				*/
				template<typename PtrTy>
				PtrTy* alignPtr(PtrTy* ptr, align_type align)
				{
					assert(align > 0 && "Alignement must be greater than 0!");
					if (align == 1)
						return ptr;

					auto ptrInt = reinterpret_cast<std::uintptr_t>(ptr);
					ptrInt += align - (ptrInt % align);
					return reinterpret_cast<PtrTy*>(ptrInt);
				}

				/*
					\brief Creates a pool if the current pool can't support an allocation of size sz.
					\return 1 if a new pool was allocated successfully. Returns 0 if no pool was allocated.
					Returns -1 if allocation of a new pool failed (too many pools)
					TL;DR: check if the result is below zero for errors.
				*/
				std::int8_t createNewPoolIfRequired(size_type sz, align_type align)
				{
					if (!firstPool)
						return createPool() ? 1 : -1;

					auto* ptr = alignPtr(static_cast<byte_type*>(allocPtr), align);
					if ((ptr + sz) > curPool->upperBound)
						return createPool() ? 1 : -1;

					return 0;
				}
		};
}