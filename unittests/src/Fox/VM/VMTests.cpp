//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : VMTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  Tests for the "VM" part of the interpreter. (Not limited to the "VM" class).
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/BC/Instruction.hpp"
#include "Fox/BC/BCBuilder.hpp"
#include "Fox/BC/BCModule.hpp"
#include "Fox/VM/VM.hpp"
#include "llvm/Support/MathExtras.h"
#include "Fox/Common/FoxTypes.hpp"
#include <sstream>

using namespace fox;

namespace {
  class VMTest : public ::testing::Test {
    public:
      BCModule theModule;
      BCBuilder builder;

      VMTest() : builder(theModule.getInstructions()) {}
  };
}

TEST_F(VMTest, StoreSmallInt) {
  FoxInt r0Value = -14242;
  FoxInt r1Value = 24000;
  builder.createStoreSmallIntInstr(1, r1Value);
  builder.createStoreSmallIntInstr(0, r0Value);
  builder.createBreakInstr();;
  VM vm(theModule);
  vm.run();
  FoxInt r0 = vm.getRegisterStack()[0];
  FoxInt r1 = vm.getRegisterStack()[1];
  EXPECT_EQ(r0, r0Value);
  EXPECT_EQ(r1, r1Value);
}

TEST_F(VMTest, IntArithmetic) {
  FoxInt r0 = 2;
  FoxInt r1 = 64;
  FoxInt r2 = -16384;
  FoxInt r3 = 16384;
  // r4 = r2 + r3 --> 0
  builder.createAddIntInstr(4, 2, 3);
  // r5 = r2 - r3 --> -32768
  builder.createSubIntInstr(5, 2, 3);
  // r6 = r0 * r1 --> 128
  builder.createMulIntInstr(6, 0, 1);
  // r7 = r3 / r1 --> 256
  builder.createDivIntInstr(7, 3, 1);
  // r8 = r2 % r1 --> 0
  builder.createModIntInstr(8, 2, 1);
  // r9 = r2 ** r0 --> 268435456
  builder.createPowIntInstr(9, 2, 0);
  // r10 = -r2 = 16384
  builder.createNegIntInstr(10, 2);
  builder.createBreakInstr();
  // Prepare the VM & Load the code
  VM vm(theModule);
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

TEST_F(VMTest, DoubleArithmetic) {
  FoxDouble r0 = -3.14;
  FoxDouble r1 = 3.333333333333;
  FoxDouble r2 = -42.42;
  FoxDouble r3 = 42500.005;
  FoxDouble r4 = 2;
  // r5 = r2 + r3 --> 42457.585
  builder.createAddDoubleInstr(5, 2, 3);
  // r6 = r2 - r3 --> -42542.425
  builder.createSubDoubleInstr(6, 2, 3);
  // r7 = r0 * r1 --> -10.46666666666562
  builder.createMulDoubleInstr(7, 0, 1);
  // r8 = r1 / r1 --> 1
  builder.createDivDoubleInstr(8, 1, 1);
  // r9 = r2 % r1 --> 0.913333333329
  builder.createModDoubleInstr(9, 2, 1);
  // r10 = r0 ** r4 --> 9.8596
  builder.createPowDoubleInstr(10, 0, 4);
  // r11 = -r2      --> 42.42
  builder.createNegDoubleInstr(11, 2);
  builder.createBreakInstr();
  // Prepare the VM & Load the code
  VM vm(theModule);

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

TEST_F(VMTest, IntComparison) {
  FoxInt r0 = 2;
  FoxInt r1 = 64;
  // r2 = (r0 == r1) --> false (0)
  builder.createEqIntInstr(2, 0, 1);
  // r3 = (r0 <= r1) --> true (1)
  builder.createLEIntInstr(3, 0, 1);
  // r4 = (r1 < r1) --> false (0)
  builder.createLTIntInstr(4, 1, 1);
  builder.createBreakInstr();
  // Prepare the VM & Load the code
  VM vm(theModule);

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

TEST_F(VMTest, DoubleComparison) {
  FoxDouble r0 = -3.14;
  FoxDouble r1 = 3.333333333333;
  // r2 = r1 == r1 --> true (1)
  builder.createEqDoubleInstr(2, 1, 1);
  // r3 = r0 <= r1 --> true (1)
  builder.createLEDoubleInstr(3, 0, 1);
  // r4 = r1 < r1  --> false (0)
  builder.createLTDoubleInstr(4, 1, 1);
  // r5 = r1 >= r0 --> true (1)
  builder.createGEDoubleInstr(5, 1, 0);
  // r6 = r1 > r1  --> false (0)
  builder.createGTDoubleInstr(6, 1, 1);
  builder.createBreakInstr();
  // Prepare the VM & Load the code
  VM vm(theModule);

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

TEST_F(VMTest, LogicOps) {
  FoxInt r0 = 0;
  FoxInt r1 = 1;
  // r2 = (r0 && r1) --> 0
  builder.createLAndInstr(2, 0, 1);
  // r3 = (r0 || r1) --> 1
  builder.createLOrInstr(3, 0, 1);
  // r4 != r1 --> 0
  builder.createLNotInstr(4, 1);
  // r5 != r0 --> 1
  builder.createLNotInstr(5, 0);
  builder.createBreakInstr();
  // Prepare the VM & Load the code
  VM vm(theModule);

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

TEST_F(VMTest, Jump) {
  // Create instructions like this:
    // 0 Jump 2
    // 1 Break
    // 2 Break    // PC should end up here
    // 3 Jump -2
    // 4 Break
  builder.createJumpInstr(2);
  builder.createBreakInstr();
  builder.createBreakInstr();
  builder.createJumpInstr(-2);
  builder.createBreakInstr();

  // Prepare the VM
  VM vm(theModule);
  
  // Run the code
  vm.run();
  // Check that the PC ended up where we expected it to.
  EXPECT_EQ(vm.getPCIndex(), 2u) << "Bad Jump";
}

TEST_F(VMTest, JumpIf) {
  FoxInt r0 = 0;
  FoxInt r1 = 1;
  // Create instructions like this:
    // 0 JumpIf r0 1  // won't jump since r0 = 0
    // 1 JumpIf r1 1  // will jump since r1 = 1
    // 2 Break    
    // 3 Break    // PC should end up here
  builder.createJumpIfInstr(0, 1);
  builder.createJumpIfInstr(1, 1);
  builder.createBreakInstr();
  builder.createBreakInstr();

  // Prepare the VM
  VM vm(theModule);
  
  // Setup initial values
  auto regs = vm.getRegisterStack();
  regs[0] = r0;
  regs[1] = r1;
  // Run the code
  vm.run();
  // Check that the PC ended up where we expected it to.
  EXPECT_EQ(vm.getPCIndex(), 3u) << "Bad JumpIf";
}

TEST_F(VMTest, JumpIfNot) {
  FoxInt r0 = 0;
  FoxInt r1 = 1;
  // Create instructions like this:
    // 0 JumpIfNot r0 1  // will jump jump since r0 = 0
    // 2 Break    
    // 1 JumpIfNot r1 1  // won't jump since r1 = 1
    // 3 Break    // PC should end up here
  builder.createJumpIfNotInstr(0, 1);
  builder.createBreakInstr();
  builder.createJumpIfNotInstr(1, 1);
  builder.createBreakInstr();

  // Prepare the VM
  VM vm(theModule);
  
  // Setup initial values
  auto regs = vm.getRegisterStack();
  regs[0] = r0;
  regs[1] = r1;
  // Run the code
  vm.run();
  // Check that the PC ended up where we expected it to.
  EXPECT_EQ(vm.getPCIndex(), 3u) << "Bad JumpIfNot";
}


TEST_F(VMTest, Casts) {
  FoxInt r0 = 42000;
  FoxInt r1 = -42;
  FoxDouble r2 = -3.3333;
  FoxDouble r3 = 3.3333;
  // IntToDouble r4 r0 -> r4 = 42000.00
  builder.createIntToDoubleInstr(4, 0);
  // IntToDouble r5 r1 -> r5 = -42.00
  builder.createIntToDoubleInstr(5, 1);
  // IntToDouble r6 r0 -> r6 = -3
  builder.createDoubleToIntInstr(6, 2);
  // IntToDouble r7 r1 -> r7 = 3
  builder.createDoubleToIntInstr(7, 3);
  builder.createBreakInstr();
  VM vm(theModule);
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

TEST_F(VMTest, Copy) {
  FoxInt r0 = 42000;
  FoxDouble r1 = -3.3333;
  // Copy r2 r0 -> r2 = r0
  builder.createCopyInstr(2, 0);
  // Copy r3 r1 -> r3 = r1
  builder.createCopyInstr(3, 1);
  builder.createBreakInstr();
  VM vm(theModule);
  auto regs = vm.getRegisterStack();
  regs[0] = r0;
  regs[1] = llvm::DoubleToBits(r1);

  vm.run();
  // Helper to get a register's value as a FoxDouble
  auto getRegAsDouble = [&](std::size_t idx) {
    return llvm::BitsToDouble(vm.getRegisterStack()[idx]);
  };
  // Helper to get a raw register value
  auto getReg = [&](std::size_t idx) {
    return static_cast<FoxInt>(vm.getRegisterStack()[idx]);
  };
  EXPECT_EQ(getReg(2), r0);
  EXPECT_DOUBLE_EQ(getRegAsDouble(3), r1);
}