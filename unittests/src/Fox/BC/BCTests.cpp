//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  Various Bytecode-related tests
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/BC/Instruction.hpp"
#include "Fox/BC/BCBuilder.hpp"
#include "Fox/BC/BCModule.hpp"
#include "Fox/BC/BCUtils.hpp"
#include "Fox/Common/FoxTypes.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/ArrayRef.h"
#include <sstream>

using namespace fox;

TEST(OpcodeTest, ToString) {
  Opcode a = Opcode::StoreSmallInt;
  Opcode b = Opcode::NoOp;
  Opcode c = Opcode::LAnd;
  Opcode illegal = static_cast<Opcode>(255);

  const char* strA = toString(a);
  const char* strB = toString(b);
  const char* strC = toString(c);
  const char* strIllegal = toString(illegal);

  EXPECT_NE(strA, nullptr);
  EXPECT_NE(strB, nullptr);
  EXPECT_NE(strC, nullptr);
  EXPECT_EQ(strIllegal, nullptr);

  EXPECT_STRCASEEQ(strA, "StoreSmallInt");
  EXPECT_STRCASEEQ(strB, "NoOp");
  EXPECT_STRCASEEQ(strC, "LAnd");
}

TEST(InstructionDumpTest, DumpInstructionsTest) {
  // Create a series of instructions 
  BCModuleBuilder builder;
  builder
    .createNoOpInstr()
    .createAddIntInstr(0, 1, 2)
    .createLNotInstr(42, 84)
    .createStoreSmallIntInstr(0, -4242)
    .createJumpInstr(-30000);
  // Check that we have the correct number of instructions
  InstructionBuffer& instrs = builder.getModule().getInstructionBuffer();
  ASSERT_EQ(instrs.size(), 5u) << "Broken BCModuleBuilder?";
  // Dump to a stringstream
  std::stringstream ss;
  dumpInstructions(ss, instrs);
  // Compare strings
  EXPECT_EQ(ss.str(),
    "NoOp\n"
    "AddInt 0 1 2\n"
    "LNot 42 84\n"
    "StoreSmallInt 0 -4242\n"
    "Jump -30000");
}

TEST(BCBuilderTest, TernaryInstr) {
  BCModuleBuilder builder;
  // Create an Ternary instr
  Instruction instr = builder.createAddIntInstr(42, 84, 126).getLastInstr();
  // Check if it was encoded as expected.
  EXPECT_EQ(instr.opcode, Opcode::AddInt);
  EXPECT_EQ(instr.AddInt.arg0, 42);
  EXPECT_EQ(instr.AddInt.arg1, 84);
  EXPECT_EQ(instr.AddInt.arg2, 126);
}

// Test for Binary Instrs with two 8 bit args.
TEST(BCBuilderTest, SmallBinaryInstr) {
  BCModuleBuilder builder;
  // Create an Small Binary instr
  Instruction instr = builder.createLNotInstr(42, 84).getLastInstr();
  // Check if it was encoded as expected.
  EXPECT_EQ(instr.opcode, Opcode::LNot);
  EXPECT_EQ(instr.LNot.arg0, 42);
  EXPECT_EQ(instr.LNot.arg1, 84);
}

// Test for Binary Instrs with one 8 bit arg and one 16 bit arg.
TEST(BCBuilderTest, BinaryInstr) {
  BCModuleBuilder builder;
  // Create a Binary instr
  Instruction instr = builder.createStoreSmallIntInstr(42, 16000).getLastInstr();
  // Check if was encoded as expected.
  EXPECT_EQ(instr.opcode, Opcode::StoreSmallInt);
  EXPECT_EQ(instr.StoreSmallInt.arg0, 42);
  EXPECT_EQ(instr.StoreSmallInt.arg1, 16000);
}

TEST(BCBuilderTest, UnaryInstr) {
  BCModuleBuilder builder;
  // Create unary instrs (this one uses a signed value)
  Instruction positive_instr = builder.createJumpInstr(30000).getLastInstr();
  Instruction negative_instr = builder.createJumpInstr(-30000).getLastInstr();
  // Check the positive one
  {
    EXPECT_EQ(positive_instr.opcode, Opcode::Jump);
    EXPECT_EQ(positive_instr.Jump.arg, 30000);
  }
  // Check the negative one
  {
    EXPECT_EQ(negative_instr.opcode, Opcode::Jump);
    EXPECT_EQ(negative_instr.Jump.arg, -30000);
  }
}

TEST(BCModuleTest, instr_iterator) {
  // Create some instructions in the builder
  BCModuleBuilder builder;
  builder.createBreakInstr();
  builder.createNoOpInstr();
  builder.createAddIntInstr(0, 0, 0);
  builder.createAddDoubleInstr(0, 0, 0);
  // Create a vector of the expected opcodes
  SmallVector<Opcode, 4> expectedOps;
  expectedOps.push_back(Opcode::Break);
  expectedOps.push_back(Opcode::NoOp);
  expectedOps.push_back(Opcode::AddInt);
  expectedOps.push_back(Opcode::AddDouble);
  // Get the module
  BCModule& theModule = builder.getModule();
  // Check that the order matches what we expect, and that
  // iteration is successful.
  auto it = theModule.instrs_begin();
  auto end = theModule.instrs_end();
  {
    int idx = 0;
    for (; it != end; ++it) {
      ASSERT_EQ(it->opcode, expectedOps[idx++]);
    }
  }
  // Check that it == end
  ASSERT_EQ(it, theModule.instrs_end());
  // Check that --it == back
  ASSERT_EQ(--it, theModule.instrs_back());
  // Check that .back is indeed AddDouble
  ASSERT_EQ(theModule.instrs_back()->opcode, Opcode::AddDouble);
}