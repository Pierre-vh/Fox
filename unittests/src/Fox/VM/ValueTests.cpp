//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : ValueTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  Tests for the Value class of the VM component.
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/Common/Typedefs.hpp"
#include "Fox/VM/Value.hpp"

using namespace fox;

TEST(ValueTest, Constructors) {
  EXPECT_TRUE(Value().isNull());
  EXPECT_TRUE(Value(FoxInt(0)).isInt());
  EXPECT_TRUE(Value(FoxDouble(0)).isDouble());
  EXPECT_TRUE(Value(FoxChar(0)).isChar());
}