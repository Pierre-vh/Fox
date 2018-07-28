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
	// Make it alignement-aware
	// Allow multiple allocations ("count" arg to allocate/deallocate)

#pragma once

#include <cstddef>
#include <memory>
#include <cassert>
#include <cstdint>
#include <type_traits>

namespace fox
{
	// Helper classes
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
		Allocation is REALLY fast, and deallocation too, at the cost of a less control over memory allocated.
		This is useful for allocating lots of long lived object (such as AST Nodes)
		Note: This class does not strive to be thread-safe. Be careful with that!
	*/
	template<std::size_t poolSize = KiloByte<64>::value, std::uint16_t maxPools = 32>
	class LinearAllocator
	{
		public:
			using size_type = std::size_t;
			using byte_type = unsigned char;

			// Assertions
			static_assert(maxPools >= 1, "You must allow at least 1 pool to be created ! (maxPools must be >= 1)");
			static_assert(poolSize >= KiloByte<1>::value, "Poolsize cannot be smaller than 1kb");

		private:

			/*
				\brief A Single Pool.
			*/
			struct Pool
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
				\brief Constructor. Does some checks and calls setup. Setup will throw if the first pool can't be created!
			*/
			LinearAllocator()
			{
				// Quick tests to see if someone isn't trying to fool us.
				static_assert(maxPools > 1, "Not enough space to allocate first pool !");
				setup();
			}

			/*
				\brief	Setup the Allocator for use. Automatically called by the ctor.
				Throws if the pool can't be created.
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
				destroy();
				setup();
			}

			/*
				\brief Allocates (size) bytes of memory, bumping the pointer by (size) bytes.
				\param size The size of the chunk of memory you want to allocate in bytes.
				\returns Your chunk of memory, nullptr if the allocator can't allocate any more memory.
			*/
			void* allocate(size_type size)
			{
				// if the object is too big to be allocated, return nullptr.
				if (!doesObjectFit(size))
					return nullptr;

				// if the allocator failed to create a new pool when required, return nullptr
				if (createNewPoolIfRequired(size) < 0)
					return nullptr;

				// Else, if everything's alright, go for it.
				assert(allocPtr && "AllocPtr cannot be null at this stage.");
				auto tmp = allocPtr;
				allocPtr = static_cast<byte_type*>(allocPtr) + size;
				return tmp;
			}

			/*
				\brief Templated version of allocate which uses sizeof() to call the base allocate. See non-templated allocate overload for more details.
			*/
			template<typename DataTy>
			auto allocate()
			{
				return static_cast<typename std::remove_all_extents<DataTy>::type*>(allocate(sizeof(DataTy)));
			}

			/*
				\brief Deallocates the pointer. This just zeroes the memory unless it's the last object allocated, then, it's actually freed.
				\param ptr The pointer which holds the chunk of memory you own.
				\param sz The size of the chunk of memory in bytes.
			*/
			void deallocate(void* ptr, size_type sz)
			{
				// set everything to zero
				memset(ptr, 0, sz);

				// Decrease the pointer if we can.
				if (allocPtr == (static_cast<byte_type*>(ptr) + sz))
					allocPtr = static_cast<byte_type*>(allocPtr) - sz;
			}

			/*
				\brief Templated version of deallocate which uses sizeof() to call the base deallocate.
			*/
			template<typename DataTy>
			void deallocate(DataTy* ptr)
			{
				deallocate(ptr, sizeof(DataTy));
			}

			/*
				\brief Destroys every pool.
			*/
			void destroy()
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
				\brief Destructor (just calls destroy())
			*/
			~LinearAllocator()
			{
				destroy();
			}

			/*
				\returns True if the object will fit in a pool, false otherwise.
			*/
			bool doesObjectFit(size_type sz) const
			{
				return (poolSize >= sz);
			}

			/*
				\brief Deletes the most recent pool.
			*/
			void popPool()
			{
				// if the current pool is not the first one
				if (curPool != firstPool.get())
				{
					assert(curPool->previous && "CurPool doesn't have a previous pool?");
					curPool = curPool->previous;
					// Destroy the pool
					curPool->next.reset();
					allocPtr = curPool->data;
					poolCount--;
				}
				else
				{
					assert(!curPool->previous && "CurPool shouldn't have a previous pool if it's the first one!");
					firstPool.reset();
					curPool = nullptr;
					poolCount = 0;
					allocPtr = nullptr;
				}
			}

			/*
				\returns True if we can't allocate any more pools.
			*/
			bool canCreateMorePools() const
			{
				return (poolCount < maxPools);
			}

			/*
				\brief Creates a pool. Will create the first one if that is not done yet, else, it'll just add another one at the end of the linked list of pools.
			*/
			bool createPool()
			{
				if (!canCreateMorePools())
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
				return true;
			}

			/*
				\brief	Displays a detailled dump to get an overview of Allocator.
				This really just displays MaxPools and PoolSize and calls smallDump()
			*/
			void dump() const
			{
				std::cout << "MaxPools: " << maxPools << "\n";
				std::cout << "Pool Size: " << poolSize << "\n";
				smallDump();
			}

			/*
				\brief Displays a condensed dump to get an overview of Allocator.
			*/
			void smallDump() const
			{
				std::cout << "Pools: " << poolCount << "\n";
				std::cout << "Curpool address: " << (void*)curPool << "\n";
				std::cout << "AllocPtr address: " << (void*)allocPtr << "\n";
				std::cout << "Bytes in current pool: " << (std::ptrdiff_t)(((byte_type*)allocPtr) - ((byte_type*)curPool)) << "\n";
			}
		private:
			/*
				\brief Creates a pool if the current pool can't support an allocation of size sz.
				\return 1 if a new pool was allocated successfully. Returns 0 if no pool was allocated. Returns -1 if allocation of a new pool failed.
				Generally you'll just check if the result is below zero for errors.
			*/
			std::int8_t createNewPoolIfRequired(size_type sz)
			{
				// If there's no pool, allocate one.
				if (!firstPool)
					return createPool() ? 1 : -1;

				// If the current pool can't hold the data, create a new one.
				else if (((static_cast<byte_type*>(allocPtr) + sz) > curPool->upperBound))
					return createPool() ? 1 : -1;

				return 0;
			}
	};
}