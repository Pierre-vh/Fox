//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : ObjectsTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/Common/Objects.hpp"

using namespace fox;

TEST(ObjectTest, RTTI) {
  StringObject object;
  EXPECT_EQ(object.getKind(), ObjectKind::StringObject);
  EXPECT_TRUE(StringObject::classof(&object));
}

TEST(ObjectTest, stringObject) {
  static constexpr char fooStr[] = "Foo is better than Bar!";
  StringObject emptyObj;
  StringObject fooObj(fooStr);

  EXPECT_EQ(emptyObj.str(), "");
  EXPECT_EQ(fooObj.str(), fooStr);
}