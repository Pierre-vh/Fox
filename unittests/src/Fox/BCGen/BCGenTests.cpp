//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCGenTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  Tests for the BCGen class
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/BC/BCModule.hpp"
#include "Fox/BCGen/BCGen.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/SourceManager.hpp"
#include "llvm/ADT/ArrayRef.h"
#include <unordered_set>

using namespace fox;

namespace {
  class BCGenTest : public ::testing::Test {
    public:
      BCGenTest() : 
        diags(srcMgr, std::cout), ctxt(srcMgr, diags), bcGen(ctxt, theModule) {}
      
      SourceManager srcMgr;
      DiagnosticEngine diags;
      ASTContext ctxt;
      BCModule theModule;
      BCGen bcGen;
  };
}

TEST_F(BCGenTest, intConstantsUniqueness) {
  static constexpr FoxInt num_constants_to_insert = 4096;
  const FoxInt beg = -4096;
  FoxInt value = beg;

  std::unordered_set<constant_id_t> ids;
  for (std::size_t idx = 0; idx < num_constants_to_insert; ++idx) {
    auto id = bcGen.getConstantID(value);
    auto result = ids.insert(id);
    ASSERT_TRUE(result.second) 
      << "constant " << value << " with id " << id 
      << " already existed in the map";
    value+=2;
  }
  ASSERT_EQ(ids.size(), num_constants_to_insert)
    << "incorrect number of IDs generated";
  // Check that they've been all inserted in the correct order in the vector.
  value = beg;
  for (auto constant : theModule.getIntConstants()) {
    ASSERT_EQ(constant, value);
    value += 2;
  }
  // Check that we can fetch each constant
  value = beg;
  for (std::size_t idx = 0; idx < num_constants_to_insert; ++idx) {
    auto id = bcGen.getConstantID(value);
    auto result = ids.find(id);
    ASSERT_NE(result, ids.end()) 
      << "constant '" << value 
      << "' was assigned a new id (" << id << ")";
    ASSERT_EQ(theModule.getIntConstant(id), value);
    value += 2;
  }
}

TEST_F(BCGenTest, doubleConstantsUniqueness) {
  static constexpr FoxInt num_constants_to_insert = 4096;
  const FoxDouble beg = -num_constants_to_insert;
  FoxDouble value = beg;

  std::unordered_set<constant_id_t> ids;
  for (std::size_t idx = 0; idx < num_constants_to_insert; ++idx) {
    auto id = bcGen.getConstantID(value);
    auto result = ids.insert(id);
    ASSERT_TRUE(result.second) 
      << "constant " << value << " with id " << id 
      << " already existed in the map";
    value+=2;
  }
  ASSERT_EQ(ids.size(), num_constants_to_insert)
    << "incorrect number of IDs generated";
  // Check that they've been all inserted in the correct order in the vector.
  value = beg;
  for (auto constant : theModule.getDoubleConstants()) {
    ASSERT_EQ(constant, value);
    value += 2;
  }
  // Check that we can fetch each constant
  value = beg;
  for (std::size_t idx = 0; idx < num_constants_to_insert; ++idx) {
    auto id = bcGen.getConstantID(value);
    auto result = ids.find(id);
    ASSERT_NE(result, ids.end()) 
      << "constant '" << value 
      << "' was assigned a new id (" << id << ")";
    ASSERT_EQ(theModule.getDoubleConstant(id), value);
    value += 2;
  }
}

// TODO: Rewrite this test to be more robust, right now it just
// adds 'a' at the end of the string for each iteration.
TEST_F(BCGenTest, stringConstantsUniqueness) {
  static constexpr std::size_t num_constants_to_insert = 256;
  const std::string beg = "\0";
  std::string value = beg;

  std::unordered_set<constant_id_t> ids;
  for (std::size_t idx = 0; idx < num_constants_to_insert; ++idx) {
    auto id = bcGen.getConstantID(value);
    auto result = ids.insert(id);
    ASSERT_TRUE(result.second) 
      << "constant " << value << " with id " << id 
      << " already existed in the map";
    value += "a";
  }
  ASSERT_EQ(ids.size(), num_constants_to_insert)
    << "incorrect number of IDs generated";
  // Check that they've been all inserted in the correct order in the vector.
  value = beg;
  for (auto constant : theModule.getStringConstants()) {
    ASSERT_EQ(constant, value);
    value += "a";
  }
  // Check that we can fetch each constant
  value = beg;
  for (std::size_t idx = 0; idx < num_constants_to_insert; ++idx) {
    auto id = bcGen.getConstantID(value);
    auto result = ids.find(id);
    ASSERT_NE(result, ids.end()) 
      << "constant '" << value 
      << "' was assigned a new id (" << id << ")";
    ASSERT_EQ(theModule.getStringConstant(id), value);
    value += "a";
  }
}