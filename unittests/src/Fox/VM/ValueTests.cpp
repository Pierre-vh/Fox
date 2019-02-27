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
  // A Value created by the "default" constructor should always be "NullValue"
  EXPECT_TRUE(Value().isNull());
}