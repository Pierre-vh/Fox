//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : StableVectorIteratorTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  Unit tests for the StableVectorIterator
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/Common/StableVectorIterator.hpp"
#include <vector>

using namespace fox;

using VectorType = std::vector<int>;
using SVI = StableVectorIterator<VectorType>;
using SVCI = StableVectorConstIterator<VectorType>;

#define STOP_IF_FAILED(x) if(HasNonfatalFailure()) FAIL() << x

static VectorType makeVector() {
  return VectorType({1, 2, 3});
}

TEST(StableVectorIteratorTest, basicTest1) {
  auto vector = makeVector();
  auto iter = SVI(vector);
  auto a = ++iter;
  auto b = iter;
  auto c = iter++;

  auto v_iter = vector.begin();
  auto v_a = ++v_iter;
  auto v_b = v_iter;
  auto v_c = v_iter++;

  EXPECT_EQ(*iter, *v_iter);
  EXPECT_EQ(*a, *v_a);
  EXPECT_EQ(*b, *v_b);
  EXPECT_EQ(*c, *v_c);
  STOP_IF_FAILED("dereference test failed");
  EXPECT_EQ(SVI(vector, v_iter), iter);
  EXPECT_EQ(SVI(vector, v_a), a);
  EXPECT_EQ(SVI(vector, v_b), b);
  EXPECT_EQ(SVI(vector, v_c), c);
  STOP_IF_FAILED("construction from vector iterator test failed");
  EXPECT_EQ(iter.getContainerIterator(), v_iter);
  EXPECT_EQ(a.getContainerIterator(), v_a);
  EXPECT_EQ(b.getContainerIterator(), v_b);
  EXPECT_EQ(c.getContainerIterator(), v_c);
  STOP_IF_FAILED("comparison with vector iterator test failed");
}

TEST(StableVectorIteratorTest, basicTest2) {
  auto vector = makeVector();
  auto iter = SVI(vector, 2);
  auto a = --iter;
  auto b = iter;
  auto c = iter--;

  auto v_iter = vector.begin()+2;
  auto v_a = --v_iter;
  auto v_b = v_iter;
  auto v_c = v_iter--;

  EXPECT_EQ(*iter, *v_iter);
  EXPECT_EQ(*a, *v_a);
  EXPECT_EQ(*b, *v_b);
  EXPECT_EQ(*c, *v_c);
  STOP_IF_FAILED("dereference test failed");
  EXPECT_EQ(SVI(vector, v_iter), iter);
  EXPECT_EQ(SVI(vector, v_a), a);
  EXPECT_EQ(SVI(vector, v_b), b);
  EXPECT_EQ(SVI(vector, v_c), c);
  STOP_IF_FAILED("construction from vector iterator test failed");
  EXPECT_EQ(iter.getContainerIterator(), v_iter);
  EXPECT_EQ(a.getContainerIterator(), v_a);
  EXPECT_EQ(b.getContainerIterator(), v_b);
  EXPECT_EQ(c.getContainerIterator(), v_c);
  STOP_IF_FAILED("comparison with vector iterator test failed");
}

TEST(StableVectorIteratorTest, basicConstTest1) {
  const auto vector = makeVector();
  auto iter = SVCI(vector);
  auto a = ++iter;
  auto b = iter;
  auto c = iter++;

  auto v_iter = vector.begin();
  auto v_a = ++v_iter;
  auto v_b = v_iter;
  auto v_c = v_iter++;

  EXPECT_EQ(*iter, *v_iter);
  EXPECT_EQ(*a, *v_a);
  EXPECT_EQ(*b, *v_b);
  EXPECT_EQ(*c, *v_c);
  STOP_IF_FAILED("dereference test failed");
  EXPECT_EQ(SVCI(vector, v_iter), iter);
  EXPECT_EQ(SVCI(vector, v_a), a);
  EXPECT_EQ(SVCI(vector, v_b), b);
  EXPECT_EQ(SVCI(vector, v_c), c);
  STOP_IF_FAILED("construction from vector iterator test failed");
  EXPECT_EQ(iter.getContainerIterator(), v_iter);
  EXPECT_EQ(a.getContainerIterator(), v_a);
  EXPECT_EQ(b.getContainerIterator(), v_b);
  EXPECT_EQ(c.getContainerIterator(), v_c);
  STOP_IF_FAILED("comparison with vector iterator test failed");
}

TEST(StableVectorIteratorTest, basicConstTest2) {
  const auto vector = makeVector();
  auto iter = SVCI(vector, 2);
  auto a = --iter;
  auto b = iter;
  auto c = iter--;

  auto v_iter = vector.begin()+2;
  auto v_a = --v_iter;
  auto v_b = v_iter;
  auto v_c = v_iter--;

  EXPECT_EQ(*iter, *v_iter);
  EXPECT_EQ(*a, *v_a);
  EXPECT_EQ(*b, *v_b);
  EXPECT_EQ(*c, *v_c);
  STOP_IF_FAILED("dereference test failed");
  EXPECT_EQ(SVCI(vector, v_iter), iter);
  EXPECT_EQ(SVCI(vector, v_a), a);
  EXPECT_EQ(SVCI(vector, v_b), b);
  EXPECT_EQ(SVCI(vector, v_c), c);
  STOP_IF_FAILED("construction from vector iterator test failed");
  EXPECT_EQ(iter.getContainerIterator(), v_iter);
  EXPECT_EQ(a.getContainerIterator(), v_a);
  EXPECT_EQ(b.getContainerIterator(), v_b);
  EXPECT_EQ(c.getContainerIterator(), v_c);
  STOP_IF_FAILED("comparison with vector iterator test failed");
}

TEST(StableVectorIteratorTest, pastTheEnd) {
  auto vector = makeVector();
  auto end_idx = SVI(vector, vector.size());
  auto end_it = SVI(vector, vector.end());
  EXPECT_EQ(end_idx.getContainerIterator(), vector.end());
  EXPECT_EQ(end_it.getContainerIterator(), vector.end());
}

TEST(StableVectorIteratorTest, stability) {
  auto vector = makeVector();
  auto iter = SVI(vector, 2);
  // Get a past-the-end iterator
  auto pte = SVI(vector, vector.end());
  // to check for realloc
  auto old_dat = vector.data();
  ASSERT_EQ(*iter, *(vector.begin()+2));
  ASSERT_EQ(pte.getContainerIterator(), vector.end());
  // Make sure to trigger a reallocation
  vector.shrink_to_fit();
  vector.push_back(4);
  vector.push_back(5);
  vector.push_back(6);
  // Actually check that we had a reallocation
  ASSERT_NE(old_dat, vector.data()) 
    << "Vector did not reallocate so we cannot trust the test results";
  // Check
  EXPECT_EQ(*iter, *(vector.begin()+2))
    << "unstable iterator";
  EXPECT_EQ(*(++iter), *(vector.begin()+3))
    << "unstable iterator";
  EXPECT_EQ(*(++iter), *(vector.begin()+4))
    << "unstable iterator";
  EXPECT_EQ(pte.getContainerIterator(), vector.end()) 
    << "unstable end iter";
}

TEST(StableVectorIteratorTest, arithmetic) {
  auto vector = makeVector();
  auto iter = SVI(vector, 2);
  auto minusone = iter-1;
  auto plusone = iter+1;
  EXPECT_EQ(iter-1, minusone);
  EXPECT_EQ(iter+1, plusone);
  EXPECT_EQ(iter, minusone+1);
  EXPECT_EQ(iter, plusone-1);
  EXPECT_EQ(plusone, iter-(-1));
  iter+=1;
  ASSERT_EQ(iter, plusone);
  iter-=2;
  ASSERT_EQ(iter, minusone);
  iter-=-2;
  ASSERT_EQ(iter, plusone);
  iter+=-2;
  ASSERT_EQ(iter, minusone);
}

// < <= > >= tests
TEST(StableVectorIteratorTest, comparison) {
  auto vector = makeVector();
  auto iter = SVI(vector, 2);
  auto minusone = iter-1;
  auto plusone = iter+1;
  // <
  EXPECT_TRUE(iter < plusone);
  EXPECT_TRUE(minusone < plusone);
  EXPECT_TRUE(minusone < iter);
  STOP_IF_FAILED("< test failed");
  // <=
  EXPECT_TRUE(iter <= plusone);
  EXPECT_TRUE(minusone <= plusone);
  EXPECT_TRUE(minusone <= iter);
  EXPECT_TRUE(iter <= iter);
  EXPECT_TRUE(minusone <= minusone);
  EXPECT_TRUE(plusone <= plusone);
  STOP_IF_FAILED("<= test failed");
  // >
  EXPECT_TRUE(plusone > iter);
  EXPECT_TRUE(plusone > minusone);
  EXPECT_TRUE(iter > minusone);
  STOP_IF_FAILED("< test failed");
  // >=
  EXPECT_TRUE(plusone > iter);
  EXPECT_TRUE(plusone > minusone);
  EXPECT_TRUE(iter > minusone);
  EXPECT_TRUE(iter >= iter);
  EXPECT_TRUE(minusone >= minusone);
  EXPECT_TRUE(plusone >= plusone);
  STOP_IF_FAILED(">= test failed");
}

TEST(StableVectorIteratorTest, distance) {
  auto vector = makeVector();
  auto beg = SVI(vector);
  auto end = SVI(vector, vector.size());
  
  auto dist_svi = distance(beg, end);
  auto dist_stl = distance(vector.begin(), vector.end());
  auto dist_svci = distance(SVCI(vector, vector.begin()), 
                            SVCI(vector, vector.end()));

  EXPECT_EQ(dist_svi, dist_stl);
  EXPECT_EQ(dist_svi, dist_svci);
  EXPECT_EQ(dist_stl, dist_svci);
}