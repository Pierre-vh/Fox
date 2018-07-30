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
	// Allow multiple allocations of a single element ("count" arg)

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

		Note: This class does not strive to be thread-safe.
	*/
	template<
			std::uint32_t poolSize = KiloByte<128>::value // Allocate 128Kb pools by default
		>							
	class LinearAllocator
	{
		public:
			// Typedefs
			using size_type = std::size_t;
			using byte_type = unsigned char;
			using align_type = std::uint8_t;

			// Assertions
			static_assert(poolSize >= 64, "Poolsize cannot be smaller than 64 Bytes");

		private:
			/*
				\brief A Single Pool. They sort of act like 
				a linked list, where each pool owns the next one.
			*/
			struct Pool
			{
				// Calculate the upperBound by adding the beginning of the data + the size of the data
				Pool(Pool* previous) : upperBound(data + poolSize), previous(previous)
				{
					memset(data, 0, sizeof(data));
				}

				byte_type data[poolSize];
				const byte_type* const upperBound = nullptr;

				// Each pool owns the next one
				std::unique_ptr<Pool> next = nullptr;

				// Pools keep a reference to the last pool, so
				// we can walk back if needed
				Pool* previous = nullptr;
			};

			std::unique_ptr<Pool> firstPool = nullptr;
			Pool* curPool = nullptr;
			size_type poolCount = 0;
			void* allocPtr = nullptr;

		public:

			/*
				\brief Constructor. Calls setup.
			*/
			LinearAllocator()
			{
				setup();
			}

			/*
				\brief Setup the Allocator for use (creates the first pool)
			*/
			void setup()
			{
				if (!firstPool)
					createPool();
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
				// Check that the allocptr isn't null
				assert(allocPtr && "AllocPtr cannot be null");

				// Check if the object fits.
				// No dump needed when calling reportBadAlloc
				if (!willFitInPool(size, align))
					reportBadAlloc("Object too big (size of object is greater than size of a pool)");

				// Create a new pool if we need to do so.
				createNewPoolIfRequired(size, align);

				// Allocate the memory
				auto tmp = alignPtr(allocPtr, align);
				allocPtr = static_cast<byte_type*>(tmp) + size;
				
				assert(tmp && "Pointer cannot be null now!");

				return tmp;
			}

			/*
				\brief Templated version of allocate which uses sizeof() to call the base allocate. See non-templated allocate overload for more details.
			*/
			template<typename DataTy>
			auto allocate()
			{
				static_assert(willFitInPool(sizeof(DataTy), alignof(DataTy)),
					"Object too big for allocator");
				return static_cast<DataTy*>(allocate(sizeof(DataTy), alignof(DataTy)));
			}

			/*
				\brief Deallocates the pointer. This just zeroes the memory unless it's the last object allocated, then, it's actually freed.
				\param ptr The pointer which holds the chunk of memory you own.
				\param sz The size of the chunk of memory in bytes.
			*/
			void deallocate(void*, size_type)
			{
				// No-op, since we don't deallocate memory in this allocator, it's
				// only freed when destroyAll is called.
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
			void destroyAll()
			{
				// Free the first pool, starting a chain reaction of destructor calls!
				if (firstPool)
					firstPool.reset();

				// Set all the member variables to safe values.
				curPool = nullptr;
				poolCount = 0;
				allocPtr = nullptr;
			}

			/*
				\brief Destructor (just calls destroyAll())
			*/
			~LinearAllocator()
			{
				destroyAll();
			}

			/*
				\brief Creates a pool. Will create the first one if that is not done yet, 
				else, it'll just add another one at the end of the linked list of pools.
			*/
			void createPool()
			{
				if (curPool)
				{
					assert(firstPool && "curPool isn't null, but firstPool is ?");
					curPool->next = std::make_unique<Pool>(curPool);
					curPool = curPool->next.get();
				}
				else
				{
					assert(!firstPool && "curPool is null, but firstPool isn't?");
					firstPool = std::make_unique<Pool>(nullptr);
					curPool = firstPool.get();
				}

				poolCount++;
				allocPtr = curPool->data;
				assert(allocPtr && "allocPtr cannot be null");
			}

			/*
				\brief	Displays a detailled dump to get an overview of the state of the allocator.
			*/
			void dump(std::ostream& os = std::cout) const
			{
				os << "(Pools Size: " << poolSize << ")\n";
				os << "Pools: " << poolCount << "\n";
				os << "Curpool address: " << (void*)curPool << "\n";
				os << "AllocPtr address: " << (void*)allocPtr << "\n";
				os << "Bytes in current pool: " << getBytesInCurrentPool() << "\n";
			}

			/*
				\returns True if a pool can handle a chunk of size "sz"
				and alignement "align"
			*/
			static constexpr bool willFitInPool(size_type sz, align_type align)
			{
				// We compare to size + (align-1), to make room for padding if needed
				return (poolSize >= (sz + (align - 1)));
			}

			/*
				\brief Returns the number of pool
			*/
			size_type getPoolCount() const
			{
				return poolCount;
			}

			/*
				\brief Returns the number of bytes in the current pool
			*/
			size_type getBytesInCurrentPool() const
			{
				return (size_type)(((byte_type*)allocPtr) - ((byte_type*)curPool));
			}
		private:
			/*
				\brief Aligns a pointer.
				\returns the aligned pointer
			*/
			template<typename PtrTy>
			PtrTy* alignPtr(PtrTy* ptr, align_type align)
			{
				// Alignement related checks
				assert((align > 0) 
					&& "Alignement cannot be less or equal to zero");
				assert(((align == 1) || ((align & (align - 1)) == 0))
					&& "Alignement must be 1, or a power of 2");

				// Don't bother calculating anything if no alignement
				// is required
				if (align == 1)
					return ptr;

				auto ptrInt = reinterpret_cast<std::uintptr_t>(ptr);
				ptrInt += align - (ptrInt % align);
				return reinterpret_cast<PtrTy*>(ptrInt);
			}

			/*
				\brief Creates a pool if the current pool can't support an allocation of size sz.
				\return true if a new pool was allocated, false if not.
			*/
			bool createNewPoolIfRequired(size_type sz, align_type align)
			{
				if (!firstPool)
				{
					createPool();
					return true;
				}

				auto* ptr = alignPtr(static_cast<byte_type*>(allocPtr), align);
				if ((ptr + sz) > curPool->upperBound)
				{
					createPool();
					return true;
				}

				return false;
			}
	};
}