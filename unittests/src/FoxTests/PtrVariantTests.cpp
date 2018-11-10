//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : PtrVariantTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// (Unit) Tests for the PtrVariant
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/Common/PtrVariant.hpp"
#include <memory>
using namespace fox;

TEST(PtrVariantTests, BasicFunctionality) {
  auto someInt = std::make_unique<int>(200);
  auto someFloat = std::make_unique<float>(3.14f);
  auto someBool = std::make_unique<bool>(true);
  PtrVariant<int, float, bool> var(someInt.get());

  // Int
  ASSERT_TRUE(var.is<int>());
  EXPECT_EQ(var.get<int>(), someInt.get());
  EXPECT_FALSE(var.is<float>());
  EXPECT_FALSE(var.is<bool>());
  EXPECT_EQ(var.getIf<float>(), nullptr);
  EXPECT_EQ(var.getIf<bool>(), nullptr);

  // Float
  var.set(someFloat.get());
  ASSERT_TRUE(var.is<float>());
  EXPECT_EQ(var.get<float>(), someFloat.get());
  EXPECT_FALSE(var.is<int>());
  EXPECT_FALSE(var.is<bool>());
  EXPECT_EQ(var.getIf<int>(), nullptr);
  EXPECT_EQ(var.getIf<bool>(), nullptr);

  // Bool
  var.set(someBool.get());
  ASSERT_TRUE(var.is<bool>());
  EXPECT_EQ(var.get<bool>(), someBool.get());
  EXPECT_FALSE(var.is<int>());
  EXPECT_FALSE(var.is<float>());
  EXPECT_EQ(var.getIf<int>(), nullptr);
  EXPECT_EQ(var.getIf<float>(), nullptr);
}

TEST(PtrVariantTests, OperatorsOverload) {
  auto someInt = std::make_unique<int>(200);
  auto someFloat = std::make_unique<float>(3.14f);
  auto someBool = std::make_unique<bool>(true);
  PtrVariant<int, float, bool> var1(someInt.get());
  PtrVariant<int, float, bool> var2(someFloat.get());

  EXPECT_FALSE(var1 == var2);
  EXPECT_FALSE(var1.getOpaque() == var2);
  EXPECT_FALSE(var1.get<int>() == var2);
  EXPECT_FALSE(var1 == var2.getOpaque());
  EXPECT_FALSE(var1 == var2.get<float>());

  EXPECT_TRUE(var1 != var2);
  EXPECT_TRUE(var1.getOpaque() != var2);
  EXPECT_TRUE(var1.get<int>() != var2);
  EXPECT_TRUE(var1 != var2.getOpaque());
  EXPECT_TRUE(var1 != var2.get<float>());

  var1 = var2;  

  EXPECT_TRUE(var1 == var2);
  EXPECT_TRUE(var1.getOpaque() == var2);
  EXPECT_TRUE(var1.get<float>() == var2);
  EXPECT_TRUE(var1 == var2.getOpaque());
  EXPECT_TRUE(var1 == var2.get<float>());

  EXPECT_FALSE(var1 != var2);
  EXPECT_FALSE(var1.getOpaque() != var2);
  EXPECT_FALSE(var1.get<float>() != var2);
  EXPECT_FALSE(var1 != var2.getOpaque());
  EXPECT_FALSE(var1 != var2.get<float>());
}