//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : VM.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  Tests for the "VM" part of the interpreter. (Not limited to the "VM" class).
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/VM/Instructions.hpp"
#include "Fox/VM/InstructionBuilder.hpp"
#include "Fox/VM/VM.hpp"
#include "llvm/Support/MathExtras.h"
#include "Fox/Common/Typedefs.hpp"

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

TEST(InstructionBuilderTest, InstrBuff) {
  InstructionBuilder builder;
  builder.createNoOpInstr().createNoOpInstr();
  // Should have 2 instrs inside it
  EXPECT_EQ(builder.getInstrs().size(), 2u);
  builder.reset();
  // Should have 0 now
  EXPECT_EQ(builder.getInstrs().size(), 0u);
}

TEST(InstructionBuilderTest, TernaryInstr) {
  InstructionBuilder builder;
  // Create an Ternary instr
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

TEST(InstructionBuilderTest, SmallBinaryInstr) {
  InstructionBuilder builder;
  // Create an Small Binary instr
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

TEST(InstructionBuilderTest, BinaryInstr) {
  InstructionBuilder builder;
  // Create a Binary instr
  std::uint32_t instr = 
    builder.createStoreSmallIntInstr(42, 16000).getLastInstr();
  // Check if was encoded as expected.
  std::uint8_t op = instr & 0x000000FF;
  std::uint8_t a = (instr & 0x0000FF00) >> 8; 
  std::uint16_t d = (instr & 0xFFFF0000) >> 16;
  EXPECT_EQ(+op, +static_cast<std::uint8_t>(Opcode::StoreSmallInt));
  EXPECT_EQ(+a, 42);
  EXPECT_EQ(d, 16000);
}

TEST(InstructionBuilderTest, UnaryInstr) {
  InstructionBuilder builder;
  // Create unary instrs
  std::uint32_t positive_instr = builder.createJumpInstr(6000000).getLastInstr();
  std::uint32_t negative_instr = builder.createJumpInstr(-6000000).getLastInstr();
  // Check the positive one
  {
    std::uint8_t op = (positive_instr & 0x000000FF);
    std::int32_t arg = llvm::SignExtend32<24>((positive_instr & 0xFFFFFF00) >> 8);
    EXPECT_EQ(+op, +static_cast<std::uint8_t>(Opcode::Jump));
    EXPECT_EQ(arg, 6000000);
  }
  // Check the negative one
  {
    std::uint8_t op = (negative_instr & 0x000000FF);
    std::int32_t arg = llvm::SignExtend32<24>((negative_instr & 0xFFFFFF00) >> 8);
    EXPECT_EQ(+op, +static_cast<std::uint8_t>(Opcode::Jump));
    EXPECT_EQ(arg, -6000000);
  }
}

// This tests that we can use both signed and unsigned values in
// StoreSmallInt. 
TEST(InstructionBuilderTest, StoreSmallInt) {
  // Encoding test
  InstructionBuilder builder;
  FoxInt r0Value = -14242;
  FoxInt r1Value = 24000;
  {
    builder.createStoreSmallIntInstr(1, r1Value);
    std::int16_t theVal = (builder.getLastInstr() & 0xFFFF0000) >> 16;
    EXPECT_EQ(theVal, r1Value);
  }
  {
    builder.createStoreSmallIntInstr(0, r0Value);
    std::int16_t theVal = (builder.getLastInstr() & 0xFFFF0000) >> 16;
    EXPECT_EQ(theVal, r0Value);
  }
}

TEST(VMTest, StoreSmallInt) {
  InstructionBuilder builder;
  FoxInt r0Value = -14242;
  FoxInt r1Value = 24000;
  builder.createStoreSmallIntInstr(1, r1Value)
         .createStoreSmallIntInstr(0, r0Value)
         .createBreakInstr();
  VM vm;
  vm.load(builder.getInstrs());
  vm.run();
  FoxInt r0 = vm.getRegisterStack()[0];
  FoxInt r1 = vm.getRegisterStack()[1];
  EXPECT_EQ(r0, r0Value);
  EXPECT_EQ(r1, r1Value);
}

TEST(VMTest, IntArithmetic) {
  InstructionBuilder builder;
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
    .createBreakInstr();
  // Prepare the VM & Load the code
  VM vm;
  vm.load(builder.getInstrs());
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
}

TEST(VMTest, DoubleArithmetic) {
  InstructionBuilder builder;
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
    .createBreakInstr();
  // Prepare the VM & Load the code
  VM vm;
  vm.load(builder.getInstrs());
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
}

TEST(VMTest, IntComparison) {
  InstructionBuilder builder;
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
  VM vm;
  vm.load(builder.getInstrs());
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
  InstructionBuilder builder;
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
  VM vm;
  vm.load(builder.getInstrs());
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
  InstructionBuilder builder;
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
  VM vm;
  vm.load(builder.getInstrs());
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
    InstructionBuilder builder;
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
    VM vm;
    vm.load(builder.getInstrs());
    // Run the code
    vm.run();
    // Check that the PC ended up where we expected it to.
    EXPECT_EQ(vm.getPC(), 2u) << "Bad Jump";
  }
  // Conditional Jump test
  {
    FoxInt r0 = 0;
    FoxInt r1 = 1;
    InstructionBuilder builder;
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
    VM vm;
    vm.load(builder.getInstrs());
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
