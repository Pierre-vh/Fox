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

TEST(InstructionTest, dumpInstructions) {
  // Create a series of instructions 
  BCModuleBuilder builder;
  builder.createNoOpInstr();
  builder.createAddIntInstr(0, 1, 2);
  builder.createLNotInstr(42, 84);
  builder.createStoreSmallIntInstr(0, -4242);
  builder.createJumpInstr(-30000);;
  // Check that we have the correct number of instructions
  InstructionBuffer& instrs = builder.getModule().getInstructions();
  ASSERT_EQ(instrs.size(), 5u) << "Broken BCModuleBuilder?";
  // Dump to a stringstream
  std::stringstream ss;
  dumpInstructions(ss, instrs);
  // Compare strings
  EXPECT_EQ(ss.str(),
    " 0\t| NoOp\n"
    " 1\t| AddInt 0 1 2\n"
    " 2\t| LNot 42 84\n"
    " 3\t| StoreSmallInt 0 -4242\n"
    " 4\t| Jump -30000\n");
}

TEST(InstructionTest, isAnyJump) {
  #define TEST_JUMP(OP)\
    EXPECT_TRUE(Instruction(Opcode::OP).isAnyJump())\
    << #OP " is a jump but isAnyJump returns false";
  #define TEST_NONJUMP(OP)\
    EXPECT_FALSE(Instruction(Opcode::OP).isAnyJump())\
    << #OP " isn't a jump but isAnyJump returns true";
  //---------------------------------------------------//
  TEST_JUMP(JumpIf);
  TEST_JUMP(JumpIfNot);
  TEST_JUMP(Jump);
  TEST_NONJUMP(StoreSmallInt);
  TEST_NONJUMP(AddInt);
  TEST_NONJUMP(LNot);
  //---------------------------------------------------//
  #undef TEST_JUMP
  #undef TEST_NONJUMP
}

TEST(BCBuilderTest, TernaryInstr) {
  BCModuleBuilder builder;
  // Create an Ternary instr
  auto it = builder.createAddIntInstr(42, 84, 126);
  // Check if it was encoded as expected.
  EXPECT_EQ(it->opcode, Opcode::AddInt);
  EXPECT_EQ(it->AddInt.dest, 42);
  EXPECT_EQ(it->AddInt.lhs, 84);
  EXPECT_EQ(it->AddInt.rhs, 126);
}

// Test for Binary Instrs with two 8 bit args.
TEST(BCBuilderTest, SmallBinaryInstr) {
  BCModuleBuilder builder;
  // Create an Small Binary instr
  auto it = builder.createLNotInstr(42, 84);
  // Check if it was encoded as expected.
  EXPECT_EQ(it->opcode, Opcode::LNot);
  EXPECT_EQ(it->LNot.dest, 42);
  EXPECT_EQ(it->LNot.src, 84);
}

// Test for Binary Instrs with one 8 bit arg and one 16 bit arg.
TEST(BCBuilderTest, BinaryInstr) {
  BCModuleBuilder builder;
  // Create a Binary instr
  auto it = builder.createStoreSmallIntInstr(42, 16000);
  // Check if was encoded as expected.
  EXPECT_EQ(it->opcode, Opcode::StoreSmallInt);
  EXPECT_EQ(it->StoreSmallInt.dest, 42);
  EXPECT_EQ(it->StoreSmallInt.value, 16000);
}

TEST(BCBuilderTest, UnaryInstr) {
  BCModuleBuilder builder;
  // Create unary instrs (this one uses a signed value)
  auto positive = builder.createJumpInstr(30000);
  auto negative = builder.createJumpInstr(-30000);
  // Check the positive one
  {
    EXPECT_EQ(positive->opcode, Opcode::Jump);
    EXPECT_EQ(positive->Jump.offset, 30000);
  }
  // Check the negative one
  {
    EXPECT_EQ(negative->opcode, Opcode::Jump);
    EXPECT_EQ(negative->Jump.offset, -30000);
  }
}

TEST(BCBuilderTest, createdInstrIterators) {
  BCModuleBuilder builder;
  // Create a few instructions, checking that iterators are valid
  auto a = builder.createJumpInstr(30000);
  EXPECT_EQ(a->opcode, Opcode::Jump);
  EXPECT_EQ(a->Jump.offset, 30000);
  auto b = builder.createJumpIfInstr(5, -4200);
  EXPECT_EQ(b->opcode, Opcode::JumpIf);
  EXPECT_EQ(b->JumpIf.condReg, 5u);
  EXPECT_EQ(b->JumpIf.offset, -4200);
  auto c = builder.createDivDoubleInstr(1, 2, 3);
  EXPECT_EQ(c->opcode, Opcode::DivDouble);
  EXPECT_EQ(c->DivDouble.dest, 1u);
  EXPECT_EQ(c->DivDouble.lhs, 2u);
  EXPECT_EQ(c->DivDouble.rhs, 3u);
  // Insert a few instructions, then tests the iterators again
  builder.createBreakInstr();
  builder.createNoOpInstr();
  auto last = builder.createBreakInstr();
  // a
  EXPECT_EQ(a->opcode, Opcode::Jump);
  EXPECT_EQ(a->Jump.offset, 30000);
  // b
  EXPECT_EQ(b->opcode, Opcode::JumpIf);
  EXPECT_EQ(b->JumpIf.condReg, 5u);
  EXPECT_EQ(b->JumpIf.offset, -4200);
  // c
  EXPECT_EQ(c->opcode, Opcode::DivDouble);
  EXPECT_EQ(c->DivDouble.dest, 1u);
  EXPECT_EQ(c->DivDouble.lhs, 2u);
  EXPECT_EQ(c->DivDouble.rhs, 3u);
  // ++c should be the break instr
  EXPECT_EQ((++c)->opcode, Opcode::Break);
  BCModule& theModule = builder.getModule();
  // ++last should be equal to end
  auto end = ++BCModule::instr_iterator(last);
  EXPECT_EQ(end, theModule.instrs_end());
  // last should be equal to instrs_last
  EXPECT_EQ(last, theModule.instrs_last());
  // last should be equal to --end
  EXPECT_EQ(last, --theModule.instrs_end());
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
  ASSERT_EQ(--it, theModule.instrs_last());
  // Check that .back is indeed AddDouble
  ASSERT_EQ(theModule.instrs_last()->opcode, Opcode::AddDouble);
}