//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : VM.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  Tests for the "VM" part of the interpreter. (Not limited to the "VM" class).
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/VM/Opcode.hpp"

using namespace fox;

TEST(OpcodeTest, IsLegal) {
  OpcodeInt iLegal = 0, iIllegal = 255;
  Opcode legal = static_cast<Opcode>(iLegal);
  Opcode illegal = static_cast<Opcode>(iIllegal);

  EXPECT_TRUE(isLegalOpcode(legal));
  EXPECT_TRUE(isLegalOpcode(iLegal));

  EXPECT_FALSE(isLegalOpcode(illegal));
  EXPECT_FALSE(isLegalOpcode(iIllegal));
}
