//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : AllocatorTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// (Unit) Tests for the LinearAllocator (& other allocators in the future
// if needed)
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/Common/LinearAllocator.hpp"

using namespace fox;

// This test "spams" the allocator
// with a lot of allocations of a large object,
// testing that the memory allocated "works".
// This will 
TEST(LinearAllocatorTests, Spam) {
  #define COUNT 8192 // Number of TestObject to allocate 
  #define NUM_VALUES 16  // Number of values in the TestObject 
  // Size of this object: 16*64 bytes = 1024 bytes
  struct TestObject {
    std::uint64_t values[NUM_VALUES];
  };

  // Create an allocator with the default parameters
  LinearAllocator<> alloc;
  
  std::vector<TestObject*> objects(COUNT);

  // Allocate COUNT of theses Objects
  for (std::size_t k = 0; k < COUNT; k++) {
    auto* ptr = alloc.allocate<TestObject>();
    ASSERT_NE(ptr, nullptr) << "The allocator returned a null pointer after " << k << " allocations";
    
    // Set every value to k+y, so they all
    // have sort of a unique value.
    for (std::size_t y = 0; y < NUM_VALUES; y++)
      ptr->values[y] = k+y;

    // Place in the array
    objects[k] = ptr;
  }
  ASSERT_EQ(objects.size(), COUNT);

  // Iterate over all of them, checks that the values are correct.
  // This is done to check that values are overwritten correctly.
  for (std::size_t k = 0; k < COUNT; k++) {
    auto* ptr = objects[k];
    for (std::size_t y = 0; y < NUM_VALUES; y++)
      ASSERT_EQ(ptr->values[y], k + y);
  }

  // Deallocate
  alloc.destroyAll();
  EXPECT_EQ(alloc.getPoolCount(), 0) << "Pools weren't released";
  EXPECT_EQ(alloc.getBytesInCurrentPool(), 0) << "Something left in the pool?";
  #undef COUNT
  #undef NUM_VALUES
}

// This test checks that the memory allocated by the allocator
// is aligned correctly.
// This tests alignements of 2, 4, 8 and 16
TEST(LinearAllocatorTests, Alignement) {
  #define ALIGNED_STRUCT(NAME,ALIGN) struct alignas(ALIGN) NAME { char thing = 0; }
  ALIGNED_STRUCT(Aligned2,  2);
  ALIGNED_STRUCT(Aligned4,  4);
  ALIGNED_STRUCT(Aligned8,  8);
  ALIGNED_STRUCT(Aligned16, 16);
  #undef ALIGNED_STRUCT

  LinearAllocator<> alloc;
  #define CHECKALIGN(PTR, ALIGN) ((reinterpret_cast<std::ptrdiff_t>(PTR) % ALIGN) == 0)
  auto* a2 = alloc.allocate<Aligned2>();
  auto* a4 = alloc.allocate<Aligned4>();
  auto* a8 = alloc.allocate<Aligned8>();
  auto* a16 = alloc.allocate<Aligned16>();

  EXPECT_NE(a2, nullptr) << "Pointer was null";
  EXPECT_NE(a4, nullptr) << "Pointer was null";
  EXPECT_NE(a8, nullptr) << "Pointer was null";
  EXPECT_NE(a16, nullptr) << "Pointer was null";

  EXPECT_TRUE(CHECKALIGN(a2, 2)) << "Incorrect alignement";
  EXPECT_TRUE(CHECKALIGN(a4, 4)) << "Incorrect alignement";
  EXPECT_TRUE(CHECKALIGN(a8, 8)) << "Incorrect alignement";
  EXPECT_TRUE(CHECKALIGN(a16, 16)) << "Incorrect alignement";
  #undef TO_INT
}


TEST(LinearAllocatorTests, ManualAllocation) {
  LinearAllocator<> alloc;
  char *foo = static_cast<char*>(alloc.allocate(32));
  
  // Set all values to an index
  for (char k = 0; k < 32; k++)
    foo[k] = k;

  // Check
  for (char k = 0; k < 32; k++)
    ASSERT_EQ(foo[k], k);
  
  // Check that we have the correct number of bytes in the current pool
  EXPECT_EQ(alloc.getBytesInCurrentPool(), 32);
  
  // Deallocate, check that the deallocation was successful
  alloc.destroyAll();
  EXPECT_EQ(alloc.getPoolCount(), 0) << "Pools weren't released";
  EXPECT_EQ(alloc.getBytesInCurrentPool(), 0) << "Something left in the pool?";
}

// This test checks that an object of the size of the pool 
// fits in a pool without throwing errors.
TEST(LinearAllocatorTests, LargeObject) {
  LinearAllocator<200> alloc;
  std::uint8_t *buff = static_cast<std::uint8_t*>(alloc.allocate(200));
  EXPECT_NE(buff, nullptr) << "Buffer is null?";
  EXPECT_EQ(alloc.getBytesInCurrentPool(), 200);
  EXPECT_EQ(alloc.getPoolCount(), 1);
  
  for (std::uint8_t k = 0; k < 200; k++)
    buff[k] = k;
  
  for (std::uint8_t k = 0; k < 200; k++)
    ASSERT_EQ(buff[k], k);
  
  
  // Now, allocating just one more byte should trigger the creation of a new pool
  char *ch = static_cast<char*>(alloc.allocate(1));
  EXPECT_EQ(alloc.getBytesInCurrentPool(), 1);
  EXPECT_EQ(alloc.getPoolCount(), 2);

  // Simple test, write to that char, just to see if the memory is OK
  (*ch) = 42;

  // Deallocate, check that the deallocation was successful
  alloc.destroyAll();
  EXPECT_EQ(alloc.getPoolCount(), 0) << "Pools weren't released";
  EXPECT_EQ(alloc.getBytesInCurrentPool(), 0) << "Something left in the pool?";
}