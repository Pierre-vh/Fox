//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : VM.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  Tests for the "VM" part of the interpreter. (Not limited to the "VM" class).
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/BC/Instructions.hpp"
#include "Fox/BC/BCBuilder.hpp"
#include "Fox/BC/BCModule.hpp"
#include "Fox/VM/VM.hpp"
#include "llvm/Support/MathExtras.h"
#include "Fox/Common/FoxTypes.hpp"
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

TEST(InstructionBuilderTest, TernaryInstr) {
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
TEST(InstructionBuilderTest, SmallBinaryInstr) {
  BCModuleBuilder builder;
  // Create an Small Binary instr
  Instruction instr = builder.createLNotInstr(42, 84).getLastInstr();
  // Check if it was encoded as expected.
  EXPECT_EQ(instr.opcode, Opcode::LNot);
  EXPECT_EQ(instr.LNot.arg0, 42);
  EXPECT_EQ(instr.LNot.arg1, 84);
}

// Test for Binary Instrs with one 8 bit arg and one 16 bit arg.
TEST(InstructionBuilderTest, BinaryInstr) {
  BCModuleBuilder builder;
  // Create a Binary instr
  Instruction instr = builder.createStoreSmallIntInstr(42, 16000).getLastInstr();
  // Check if was encoded as expected.
  EXPECT_EQ(instr.opcode, Opcode::StoreSmallInt);
  EXPECT_EQ(instr.StoreSmallInt.arg0, 42);
  EXPECT_EQ(instr.StoreSmallInt.arg1, 16000);
}

TEST(InstructionBuilderTest, UnaryInstr) {
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

TEST(VMTest, StoreSmallInt) {
  BCModuleBuilder builder;
  FoxInt r0Value = -14242;
  FoxInt r1Value = 24000;
  builder.createStoreSmallIntInstr(1, r1Value)
         .createStoreSmallIntInstr(0, r0Value)
         .createBreakInstr();
  VM vm(builder.getModule());
  vm.run();
  FoxInt r0 = vm.getRegisterStack()[0];
  FoxInt r1 = vm.getRegisterStack()[1];
  EXPECT_EQ(r0, r0Value);
  EXPECT_EQ(r1, r1Value);
}

TEST(VMTest, IntArithmetic) {
  BCModuleBuilder builder;
  FoxInt r0 = 2;
  FoxInt r1 = 64;
  FoxInt r2 = -16384;
  FoxInt r3 = 16384;
  builder 
    // r4 = r2 + r3 --> 0
    .createAddIntInstr(4, 2, 3)
    // r5 = r2 - r3 --> -32768
    .createSubIntInstr(5, 2, 3)
    // r6 = r0 * r1 --> 128
    .createMulIntInstr(6, 0, 1)
    // r7 = r3 / r1 --> 256
    .createDivIntInstr(7, 3, 1)
    // r8 = r2 % r1 --> 0
    .createModIntInstr(8, 2, 1)
    // r9 = r2 ** r0 --> 268435456
    .createPowIntInstr(9, 2, 0)
    // r10 = -r2 = 16384
    .createNegIntInstr(10, 2)
    .createBreakInstr();
  // Prepare the VM & Load the code
  VM vm(builder.getModule());
  // Load the initial values
  auto regs = vm.getRegisterStack();
  regs[0] = r0;
  regs[1] = r1;
  regs[2] = r2;
  regs[3] = r3;
  // Run the code
  vm.run();
  // Helper to get a register's value as a FoxInt
  auto getReg = [&](std::size_t idx) {
    return static_cast<FoxInt>(regs[idx]);
  };
  // Check the initial values
  ASSERT_EQ(getReg(0), r0);
  ASSERT_EQ(getReg(1), r1);
  ASSERT_EQ(getReg(2), r2);
  ASSERT_EQ(getReg(3), r3);
  // Check the computed values
  EXPECT_EQ(getReg(4), 0)         << "Bad AddInt";
  EXPECT_EQ(getReg(5), -32768)    << "Bad SubInt";
  EXPECT_EQ(getReg(6), 128)       << "Bad MulInt";
  EXPECT_EQ(getReg(7), 256)       << "Bad DivInt";
  EXPECT_EQ(getReg(8), 0)         << "Bad ModInt";
  EXPECT_EQ(getReg(9), 268435456) << "Bad PowInt";
  EXPECT_EQ(getReg(10), 16384)    << "Bad NegInt";
}

TEST(VMTest, DoubleArithmetic) {
  BCModuleBuilder builder;
  FoxDouble r0 = -3.14;
  FoxDouble r1 = 3.333333333333;
  FoxDouble r2 = -42.42;
  FoxDouble r3 = 42500.005;
  FoxDouble r4 = 2;
  builder 
    // r5 = r2 + r3 --> 42457.585
    .createAddDoubleInstr(5, 2, 3)
    // r6 = r2 - r3 --> -42542.425
    .createSubDoubleInstr(6, 2, 3)
    // r7 = r0 * r1 --> -10.46666666666562
    .createMulDoubleInstr(7, 0, 1)
    // r8 = r1 / r1 --> 1
    .createDivDoubleInstr(8, 1, 1)
    // r9 = r2 % r1 --> 0.913333333329
    .createModDoubleInstr(9, 2, 1)
    // r10 = r0 ** r4 --> 9.8596
    .createPowDoubleInstr(10, 0, 4)
    // r11 = -r2      --> 42.42
    .createNegDoubleInstr(11, 2)
    .createBreakInstr();
  // Prepare the VM & Load the code
  VM vm(builder.getModule());

  auto regs = vm.getRegisterStack();
  regs[0] = llvm::DoubleToBits(r0);
  regs[1] = llvm::DoubleToBits(r1);
  regs[2] = llvm::DoubleToBits(r2);
  regs[3] = llvm::DoubleToBits(r3);
  regs[4] = llvm::DoubleToBits(r4);
  // Run the code
  vm.run();
  // Helper to get a register's value as a FoxDouble
  auto getReg = [&](std::size_t idx) {
    return llvm::BitsToDouble(vm.getRegisterStack()[idx]);
  };
  // Check the initial values
  ASSERT_EQ(getReg(0), r0);
  ASSERT_EQ(getReg(1), r1);
  ASSERT_EQ(getReg(2), r2);
  ASSERT_EQ(getReg(3), r3);
  ASSERT_EQ(getReg(4), r4);
  // Check the computed values
  EXPECT_DOUBLE_EQ(getReg(5), 42457.585)            << "Bad AddDouble";
  EXPECT_DOUBLE_EQ(getReg(6), -42542.425)           << "Bad SubDouble";
  EXPECT_DOUBLE_EQ(getReg(7), -10.46666666666562)   << "Bad MulDouble";
  EXPECT_DOUBLE_EQ(getReg(8), 1)                    << "Bad DivDouble";
  EXPECT_DOUBLE_EQ(getReg(9), -2.4200000000040021)  << "Bad ModDouble";
  EXPECT_DOUBLE_EQ(getReg(10), 9.8596)              << "Bad PowDouble";
  EXPECT_EQ(getReg(11), 42.42)                      << "Bad NegDouble";
}

TEST(VMTest, IntComparison) {
  BCModuleBuilder builder;
  FoxInt r0 = 2;
  FoxInt r1 = 64;
  builder 
    // r2 = (r0 == r1) --> false (0)
    .createEqIntInstr(2, 0, 1)
    // r3 = (r0 <= r1) --> true (1)
    .createLEIntInstr(3, 0, 1)
    // r4 = (r1 < r1) --> false (0)
    .createLTIntInstr(4, 1, 1)
    .createBreakInstr();
  // Prepare the VM & Load the code
  VM vm(builder.getModule());

  // Load the initial values
  auto regs = vm.getRegisterStack();
  regs[0] = r0;
  regs[1] = r1;
  // Run the code
  vm.run();
  // Helper to get a register's value as a FoxInt
  auto getReg = [&](std::size_t idx) {
    return static_cast<FoxInt>(regs[idx]);
  };
  // Check the initial values
  ASSERT_EQ(getReg(0), r0);
  ASSERT_EQ(getReg(1), r1);
  // Check the computed values
  EXPECT_EQ(getReg(2), 0) << "Bad EqInt";
  EXPECT_EQ(getReg(3), 1) << "Bad LEInt";
  EXPECT_EQ(getReg(4), 0) << "Bad LTInt";
}

TEST(VMTest, DoubleComparison) {
  BCModuleBuilder builder;
  FoxDouble r0 = -3.14;
  FoxDouble r1 = 3.333333333333;
  builder 
    // r2 = r1 == r1 --> true (1)
    .createEqDoubleInstr(2, 1, 1)
    // r3 = r0 <= r1 --> true (1)
    .createLEDoubleInstr(3, 0, 1)
    // r4 = r1 < r1  --> false (0)
    .createLTDoubleInstr(4, 1, 1)
    // r5 = r1 >= r0 --> true (1)
    .createGEDoubleInstr(5, 1, 0)
    // r6 = r1 > r1  --> false (0)
    .createGTDoubleInstr(6, 1, 1)
    .createBreakInstr();
  // Prepare the VM & Load the code
  VM vm(builder.getModule());

  auto regs = vm.getRegisterStack();
  regs[0] = llvm::DoubleToBits(r0);
  regs[1] = llvm::DoubleToBits(r1);
  // Run the code
  vm.run();
  // Helper to get a register's value as a FoxDouble
  auto getRegAsDouble = [&](std::size_t idx) {
    return llvm::BitsToDouble(vm.getRegisterStack()[idx]);
  };
  // Helper to get a raw register value
  auto getReg = [&](std::size_t idx) {
    return vm.getRegisterStack()[idx];
  };
  // Check the initial values
  ASSERT_EQ(getRegAsDouble(0), r0);
  ASSERT_EQ(getRegAsDouble(1), r1);
  // Check the computed values
  EXPECT_DOUBLE_EQ(getReg(2), 1)  << "Bad EqDouble";
  EXPECT_DOUBLE_EQ(getReg(3), 1)  << "Bad LEDouble";
  EXPECT_DOUBLE_EQ(getReg(4), 0)  << "Bad LTDouble";
  EXPECT_DOUBLE_EQ(getReg(5), 1)  << "Bad GEDouble";
  EXPECT_DOUBLE_EQ(getReg(6), 0)  << "Bad GTDouble";
}

TEST(VMTest, LogicOps) {
  BCModuleBuilder builder;
  FoxInt r0 = 0;
  FoxInt r1 = 1;
  builder 
    // r2 = (r0 && r1) --> 0
    .createLAndInstr(2, 0, 1)
    // r3 = (r0 || r1) --> 1
    .createLOrInstr(3, 0, 1)
    // r4 != r1 --> 0
    .createLNotInstr(4, 1)
    // r5 != r0 --> 1
    .createLNotInstr(5, 0)
    .createBreakInstr();
  // Prepare the VM & Load the code
  VM vm(builder.getModule());

  // Load the initial values
  auto regs = vm.getRegisterStack();
  regs[0] = r0;
  regs[1] = r1;
  // Run the code
  vm.run();
  // Helper to get a register's value as a FoxInt
  auto getReg = [&](std::size_t idx) {
    return static_cast<FoxInt>(regs[idx]);
  };
  // Check the initial values
  ASSERT_EQ(getReg(0), r0);
  ASSERT_EQ(getReg(1), r1);
  // Check the computed values
  EXPECT_EQ(getReg(2), 0) << "Bad LOr";
  EXPECT_EQ(getReg(3), 1) << "Bad LAnd";
  EXPECT_EQ(getReg(4), 0) << "Bad LNot";
  EXPECT_EQ(getReg(5), 1) << "Bad LNot";
}


TEST(VMTest, Jumps) {
  // Unconditional Jump test
  {
    BCModuleBuilder builder;
    // Create instructions like this:
      // 0 Jump 2
      // 1 Break
      // 2 Break    // PC should end up here
      // 3 Jump -2
      // 4 Break
    builder
      .createJumpInstr(2)
      .createBreakInstr()
      .createBreakInstr()
      .createJumpInstr(-2)
      .createBreakInstr();

    // Prepare the VM
    VM vm(builder.getModule());
  
    // Run the code
    vm.run();
    // Check that the PC ended up where we expected it to.
    EXPECT_EQ(vm.getPC(), 2u) << "Bad Jump";
  }
  // Conditional Jump test
  {
    FoxInt r0 = 0;
    FoxInt r1 = 1;
    BCModuleBuilder builder;
    // Create instructions like this:
      // 0 CondJump r0 1  // won't jump since r0 = 0
      // 1 CondJump r1 1  // will jump since r1 = 1
      // 2 Break    
      // 3 Break    // PC should end up here
    builder
      .createCondJumpInstr(0, 1)
      .createCondJumpInstr(1, 1)
      .createBreakInstr()
      .createBreakInstr();

    // Prepare the VM
    VM vm(builder.getModule());
  
    // Setup initial values
    auto regs = vm.getRegisterStack();
    regs[0] = r0;
    regs[1] = r1;
    // Run the code
    vm.run();
    // Check that the PC ended up where we expected it to.
    EXPECT_EQ(vm.getPC(), 3u) << "Bad CondJump";
  }
}

TEST(VMTest, Casts) {
  BCModuleBuilder builder;
  FoxInt r0 = 42000;
  FoxInt r1 = -42;
  FoxDouble r2 = -3.3333;
  FoxDouble r3 = 3.3333;
  builder 
    // IntToDouble r4 r0 -> r4 = 42000.00
    .createIntToDoubleInstr(4, 0)
    // IntToDouble r5 r1 -> r5 = -42.00
    .createIntToDoubleInstr(5, 1)
    // IntToDouble r6 r0 -> r6 = -3
    .createDoubleToIntInstr(6, 2)
    // IntToDouble r7 r1 -> r7 = 3
    .createDoubleToIntInstr(7, 3)
    .createBreakInstr();
  VM vm(builder.getModule());
  auto regs = vm.getRegisterStack();
  regs[0] = r0;
  regs[1] = r1;
  regs[2] = llvm::DoubleToBits(r2);
  regs[3] = llvm::DoubleToBits(r3);

  vm.run();
  // Helper to get a register's value as a FoxDouble
  auto getRegAsDouble = [&](std::size_t idx) {
    return llvm::BitsToDouble(vm.getRegisterStack()[idx]);
  };
  // Helper to get a raw register value
  auto getReg = [&](std::size_t idx) {
    return vm.getRegisterStack()[idx];
  };
  EXPECT_DOUBLE_EQ(getRegAsDouble(4), 42000.00);
  EXPECT_DOUBLE_EQ(getRegAsDouble(5), -42.00);
  EXPECT_EQ(getReg(6), -3);
  EXPECT_EQ(getReg(7), 3);
}