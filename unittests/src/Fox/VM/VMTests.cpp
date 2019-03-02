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
#include "Fox/VM/InstructionBuilder.hpp"

using namespace fox;

TEST(OpcodeTest, IsLegal) {
  std::uint8_t iLegal = 0, iIllegal = 255;
  Opcode legal = static_cast<Opcode>(iLegal);
  Opcode illegal = static_cast<Opcode>(iIllegal);

  EXPECT_TRUE(isLegalOpcode(legal));
  EXPECT_TRUE(isLegalOpcode(iLegal));

  EXPECT_FALSE(isLegalOpcode(illegal));
  EXPECT_FALSE(isLegalOpcode(iIllegal));
}

TEST(OpcodeTest, ToString) {
  Opcode illegal = static_cast<Opcode>(255);
  Opcode nop = Opcode::NoOp;
  Opcode addInt = Opcode::AddInt;

  EXPECT_EQ(toString(illegal), nullptr);
  EXPECT_STRCASEEQ(toString(nop), "NoOp");
  EXPECT_STRCASEEQ(toString(addInt), "AddInt");
}

TEST(InstructionBuilderTest, InstrBuff) {
  InstructionBuilder builder;
  builder.createNoOpInstr().createNoOpInstr();
  // Should have 2 instrs inside it
  EXPECT_EQ(builder.getInstrs().size(), 2u);
  builder.reset();
  // Should have 0 now
  EXPECT_EQ(builder.getInstrs().size(), 0u);
}

TEST(InstructionBuilderTest, ABCInstr) {
  InstructionBuilder builder;
  // Create an ABC instr
  std::uint32_t instr = builder.createAddIntInstr(42, 84, 126).getLastInstr();
  // Check if was encoded as expected.
  std::uint8_t op = instr & 0x000000FF;
  std::uint8_t a = (instr & 0x0000FF00) >> 8; 
  std::uint8_t b = (instr & 0x00FF0000) >> 16; 
  std::uint8_t c = (instr & 0xFF000000) >> 24;
  EXPECT_EQ(+op, +static_cast<std::uint8_t>(Opcode::AddInt));
  EXPECT_EQ(+a, 42);
  EXPECT_EQ(+b, 84);
  EXPECT_EQ(+c, 126);
}

TEST(InstructionBuilderTest, ABInstr) {
  InstructionBuilder builder;
  // Create an ABC instr
  std::uint32_t instr = builder.createLNotInstr(42, 84).getLastInstr();
  // Check if was encoded as expected.
  std::uint8_t op = instr & 0x000000FF;
  std::uint8_t a = (instr & 0x0000FF00) >> 8; 
  std::uint8_t b = (instr & 0x00FF0000) >> 16; 
  std::uint8_t c = (instr & 0xFF000000) >> 24;
  EXPECT_EQ(+op, +static_cast<std::uint8_t>(Opcode::LNot));
  EXPECT_EQ(+a, 42);
  EXPECT_EQ(+b, 84);
  EXPECT_EQ(+c, 0);
}

TEST(InstructionBuilderTest, ADInstr) {
  InstructionBuilder builder;
  // Create an AD instr
  std::uint32_t instr = 
    builder.createStoreSmallIntInstr(42, 42042).getLastInstr();
  // Check if was encoded as expected.
  std::uint8_t op = instr & 0x000000FF;
  std::uint8_t a = (instr & 0x0000FF00) >> 8; 
  std::uint16_t d = (instr & 0xFFFF0000) >> 16;
  EXPECT_EQ(+op, +static_cast<std::uint8_t>(Opcode::StoreSmallInt));
  EXPECT_EQ(+a, 42);
  EXPECT_EQ(d, 42042);
}

// This tests that we can use both signed and unsigned values in
// StoreSmallInt. 
//
// I'm testi,g this because I'm a bit of a noob with signedness stuff
// so I want to be sure that it works as expected. 
//  - Pierre
TEST(InstructionBuilderTest, StoreSmallInt) {
  InstructionBuilder builder;
  {
    std::int32_t positive = 42042;
    builder.createStoreSmallIntInstr(0, positive);
    std::uint16_t theVal = (builder.getLastInstr() & 0xFFFF0000) >> 16;
    EXPECT_EQ(theVal, positive);
  }
  {
    std::int16_t negative = -4242;
    builder.createStoreSmallIntInstr(0, negative);
    std::uint16_t theVal = (builder.getLastInstr() & 0xFFFF0000) >> 16;
    EXPECT_EQ(static_cast<std::int16_t>(theVal), negative);
  }
}