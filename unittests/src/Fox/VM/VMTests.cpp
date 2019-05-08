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
#include "Fox/Common/FoxTypes.hpp"
#include "Fox/Common/Objects.hpp"
#include <sstream>

using namespace fox;

namespace {
  class VMTest : public ::testing::Test {
    public:
      BCModule theModule;
      InstructionVector instrs;
      BCBuilder builder;

      VMTest() : builder(instrs) {}
  };
}

TEST_F(VMTest, StoreSmallInt) {
  FoxInt r0Value = -14242;
  FoxInt r1Value = 24000;
  builder.createStoreSmallIntInstr(1, r1Value);
  builder.createStoreSmallIntInstr(0, r0Value);
  builder.createRetVoidInstr();;
  VM vm(theModule);
  vm.run(instrs);
  FoxInt r0 = vm.getRegisterStack()[0].intVal;
  FoxInt r1 = vm.getRegisterStack()[1].intVal;
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
  builder.createRetVoidInstr();
  // Prepare the VM & Load the code
  VM vm(theModule);
  // Load the initial values
  auto regs = vm.getRegisterStack();
  regs[0].intVal = r0;
  regs[1].intVal = r1;
  regs[2].intVal = r2;
  regs[3].intVal = r3;
  // Run the code
  vm.run(instrs);
  // Helper to get a register's value as a FoxInt
  auto getReg = [&](std::size_t idx) {
    return regs[idx].intVal;
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
  builder.createRetVoidInstr();
  // Prepare the VM & Load the code
  VM vm(theModule);

  auto regs = vm.getRegisterStack();
  regs[0].doubleVal = r0;
  regs[1].doubleVal = r1;
  regs[2].doubleVal = r2;
  regs[3].doubleVal = r3;
  regs[4].doubleVal = r4;
  // Run the code
  vm.run(instrs);
  // Helper to get a register's value as a FoxDouble
  auto getReg = [&](std::size_t idx) {
    return vm.getRegisterStack()[idx].doubleVal;
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
  builder.createRetVoidInstr();
  // Prepare the VM & Load the code
  VM vm(theModule);

  // Load the initial values
  auto regs = vm.getRegisterStack();
  regs[0].intVal = r0;
  regs[1].intVal = r1;
  // Run the code
  vm.run(instrs);
  // Helper to get a register's value as a FoxInt
  auto getReg = [&](std::size_t idx) {
    return regs[idx].intVal;
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
  builder.createRetVoidInstr();
  // Prepare the VM & Load the code
  VM vm(theModule);

  auto regs = vm.getRegisterStack();
  regs[0].doubleVal = r0;
  regs[1].doubleVal = r1;
  // Run the code
  vm.run(instrs);
  // Helper to get a register's value as a FoxDouble
  auto getRegAsDouble = [&](std::size_t idx) {
    return vm.getRegisterStack()[idx].doubleVal;
  };
  // Helper to get a raw register value
  auto getReg = [&](std::size_t idx) {
    return vm.getRegisterStack()[idx].raw;
  };
  // Check the initial values
  ASSERT_EQ(getRegAsDouble(0), r0);
  ASSERT_EQ(getRegAsDouble(1), r1);
  // Check the computed values
  EXPECT_DOUBLE_EQ(getReg(2), true)  << "Bad EqDouble";
  EXPECT_DOUBLE_EQ(getReg(3), true)  << "Bad LEDouble";
  EXPECT_DOUBLE_EQ(getReg(4), false)  << "Bad LTDouble";
  EXPECT_DOUBLE_EQ(getReg(5), true)  << "Bad GEDouble";
  EXPECT_DOUBLE_EQ(getReg(6), false)  << "Bad GTDouble";
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
  builder.createRetVoidInstr();
  // Prepare the VM & Load the code
  VM vm(theModule);

  // Load the initial values
  auto regs = vm.getRegisterStack();
  regs[0].intVal = r0;
  regs[1].intVal = r1;
  // Run the code
  vm.run(instrs);
  // Helper to get a register's value as a FoxInt
  auto getReg = [&](std::size_t idx) {
    return regs[idx].intVal;
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
  builder.createRetVoidInstr();
  builder.createRetVoidInstr();
  builder.createJumpInstr(-2);
  builder.createRetVoidInstr();

  // Prepare the VM
  VM vm(theModule);
  
  // Run the code
  vm.run(instrs);
  // Check that the PC ended up where we expected it to.
  EXPECT_EQ(vm.getPC(), instrs.begin()+2) << "Bad Jump";
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
  builder.createRetVoidInstr();
  builder.createRetVoidInstr();

  // Prepare the VM
  VM vm(theModule);
  
  // Setup initial values
  auto regs = vm.getRegisterStack();
  regs[0].intVal = r0;
  regs[1].intVal = r1;
  // Run the code
  vm.run(instrs);
  // Check that the PC ended up where we expected it to.
  EXPECT_EQ(vm.getPC(), instrs.begin()+3u) << "Bad JumpIf";
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
  builder.createRetVoidInstr();
  builder.createJumpIfNotInstr(1, 1);
  builder.createRetVoidInstr();

  // Prepare the VM
  VM vm(theModule);
  
  // Setup initial values
  auto regs = vm.getRegisterStack();
  regs[0].intVal = r0;
  regs[1].intVal = r1;
  // Run the code
  vm.run(instrs);
  // Check that the PC ended up where we expected it to.
  EXPECT_EQ(vm.getPC(), instrs.begin()+3u) << "Bad JumpIfNot";
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
  builder.createRetVoidInstr();
  VM vm(theModule);
  auto regs = vm.getRegisterStack();
  regs[0].intVal = r0;
  regs[1].intVal = r1;
  regs[2].doubleVal = r2;
  regs[3].doubleVal = r3;

  vm.run(instrs);
  // Helper to get a register's value as a FoxDouble
  auto getDbl = [&](std::size_t idx) {
    return vm.getRegisterStack()[idx].doubleVal;
  };
  // Helper to get a raw register value
  auto getInt = [&](std::size_t idx) {
    return vm.getRegisterStack()[idx].intVal;
  };
  EXPECT_DOUBLE_EQ(getDbl(4), FoxDouble(r0));
  EXPECT_DOUBLE_EQ(getDbl(5), FoxDouble(r1));
  EXPECT_EQ(getInt(6), FoxInt(r2));
  EXPECT_EQ(getInt(7), FoxInt(r3));
}

TEST_F(VMTest, Copy) {
  FoxInt r0 = 42000;
  FoxDouble r1 = -3.3333;
  // Copy r2 r0 -> r2 = r0
  builder.createCopyInstr(2, 0);
  // Copy r3 r1 -> r3 = r1
  builder.createCopyInstr(3, 1);
  builder.createRetVoidInstr();
  VM vm(theModule);
  auto regs = vm.getRegisterStack();
  regs[0].intVal = r0;
  regs[1].doubleVal = r1;

  vm.run(instrs);
  // Helper to get a register's value as a FoxDouble
  auto getRegAsDouble = [&](std::size_t idx) {
    return vm.getRegisterStack()[idx].doubleVal;
  };
  // Helper to get a raw register value
  auto getReg = [&](std::size_t idx) {
    return vm.getRegisterStack()[idx].intVal;
  };
  EXPECT_EQ(getReg(2), r0);
  EXPECT_DOUBLE_EQ(getRegAsDouble(3), r1);
}

TEST_F(VMTest, LoadIntK) {
  FoxInt k0 = std::numeric_limits<FoxInt>::max();
  FoxInt k1 = std::numeric_limits<FoxInt>::min();
  FoxInt k2 = 1;

  // Load the constants into the constant table
  theModule.addIntConstant(k0);
  theModule.addIntConstant(k1);
  theModule.addIntConstant(k2);

  // load k0 into r0
  builder.createLoadIntKInstr(0, 0);
  // load k1 into r1
  builder.createLoadIntKInstr(1, 1);
  // load k2 into r2
  builder.createLoadIntKInstr(2, 2);
  builder.createRetVoidInstr();
  // Prepare the VM & Load the code
  VM vm(theModule);

  // Run the code
  vm.run(instrs);
  // Helper to get a register's value as a FoxInt
  auto getReg = [&](std::size_t idx) {
    return vm.getRegisterStack()[idx].intVal;
  };
  // Check that the values are correct
  EXPECT_EQ(getReg(0), k0);
  EXPECT_EQ(getReg(1), k1);
  EXPECT_EQ(getReg(2), k2);
}

TEST_F(VMTest, LoadDoubleK) {
  FoxDouble k0 = std::numeric_limits<FoxDouble>::max();
  FoxDouble k1 = std::numeric_limits<FoxDouble>::min();
  FoxDouble k2 = 0.3333333333;

  // Load the constants into the constant table
  theModule.addDoubleConstant(k0);
  theModule.addDoubleConstant(k1);
  theModule.addDoubleConstant(k2);

  // load k0 into r0
  builder.createLoadDoubleKInstr(0, 0);
  // load k1 into r1
  builder.createLoadDoubleKInstr(1, 1);
  // load k2 into r2
  builder.createLoadDoubleKInstr(2, 2);
  builder.createRetVoidInstr();
  // Prepare the VM & Load the code
  VM vm(theModule);

  // Run the code
  vm.run(instrs);
  // Helper to get a register's value as a FoxDouble
  auto getReg = [&](std::size_t idx) {
    return vm.getRegisterStack()[idx].doubleVal;
  };
  // Check that the values are correct
  EXPECT_EQ(getReg(0), k0);
  EXPECT_EQ(getReg(1), k1);
  EXPECT_EQ(getReg(2), k2);
}

TEST_F(VMTest, NewStringAndLoadStringK) {
  std::string foo = "Foo is awesome!";
  std::string bar = "...so is Bar!";

  // Load the constants into the constant table
  theModule.addStringConstant(foo);
  theModule.addStringConstant(bar);

  // load k0 into r0
  builder.createLoadStringKInstr(0, 0);
  // load k1 into r1
  builder.createLoadStringKInstr(1, 1);
  // create a blank string in r2
  builder.createNewStringInstr(2);
  builder.createRetVoidInstr();

  VM vm(theModule);
  vm.run(instrs);

  // Helper to get a register's value as a StringObject*
  auto getReg = [&](std::size_t idx) {
    return (StringObject*)vm.getRegisterStack()[idx].object;
  };

  // Check that the values are correct
  EXPECT_EQ(getReg(0)->str(), foo);
  EXPECT_EQ(getReg(1)->str(), bar);
  EXPECT_EQ(getReg(2)->str(), "");
}

TEST_F(VMTest, RetRetVoid) {
  // f1 = just a RetVoid
  BCFunction& f1 = theModule.createFunction();
  f1.createBCBuilder().createRetVoidInstr();
  VM vm(theModule);
  EXPECT_EQ(vm.call(f1), VM::Register()) << "RetVoid returned something non-null";
  // f2 = stores something in r1 and and returns 
  // the value in r1
  BCFunction& f2 = theModule.createFunction();
  {
    BCBuilder builder = f2.createBCBuilder();
    builder.createStoreSmallIntInstr(1, 526);
    builder.createRetInstr(1);
  }
  // Compare values
  auto ret = vm.call(f2);
  auto regstack = vm.getRegisterStack();
  EXPECT_EQ(ret.intVal, 526);
}

TEST_F(VMTest, runFuncWithArgs) {
  using Register = VM::Register;
  // Create a function that takes 3 parameters
  // Parameters should be in r0, r1 and r2
  BCFunction& fn = theModule.createFunction();
  BCBuilder builder = fn.createBCBuilder();
  // Create a few instructions so r0 = r1 + r2
  // and r1 = r1 + r2
  // the return r2
  builder.createAddIntInstr(0, 1, 2);
  builder.createAddIntInstr(1, 1, 2);
  builder.createRetInstr(2);

  VM vm(theModule);
  // Initial values: -5, 0, 5
  FoxInt a0 = -5;
  FoxInt a1 = 0;
  FoxInt a2 = 5;
  Register args[3] = {Register(a0), Register(a1), Register(a2)};
  Register result = vm.call(fn, args);
  // Check that the call went as expected
  EXPECT_EQ(result.intVal, a2);
  EXPECT_EQ(args[0].intVal, a1 + a2);
  EXPECT_EQ(args[1].intVal, a1 + a2);
}

TEST_F(VMTest, loadFunc) {
  BCFunction& fn = theModule.createFunction();
  BCBuilder builder = fn.createBCBuilder();
  ASSERT_EQ(fn.getID(), 0u);
  builder.createLoadFuncInstr(0, 0);
  builder.createRetVoidInstr();
  VM vm(theModule);
  vm.call(fn);
  // Helper to get a register's value as a BCFunction*
  auto getReg = [&](std::size_t idx) {
    return vm.getRegisterStack()[idx].funcRef.getBCFunction();
  };
  // Check that the function was loaded correctly
  EXPECT_EQ(getReg(0), &fn);
}

TEST_F(VMTest, loadBuiltinFunc) {
  BCFunction& fn = theModule.createFunction();
  BCBuilder builder = fn.createBCBuilder();
  ASSERT_EQ(fn.getID(), 0u);
  builder.createLoadBuiltinFuncInstr(0, BuiltinID::printBool);
  builder.createRetVoidInstr();
  VM vm(theModule);
  vm.call(fn);
  // Helper to get a register's value as a BuiltinID
  auto getReg = [&](std::size_t idx) {
    return vm.getRegisterStack()[idx].funcRef.getBuiltinID();
  };
  // Check that the builtin was loaded correctly
  EXPECT_EQ(getReg(0), BuiltinID::printBool);
}

TEST_F(VMTest, call) {
  // Create 2 functions, f0 and f1
  // f0 just calls f1
  BCFunction& f0 = theModule.createFunction();
  ASSERT_EQ(f0.getID(), 0u);
  FoxInt r1 = 4;
  FoxInt r2 = 8;
  {
    BCBuilder builder = f0.createBCBuilder();
    // Setup the call so r0 = base = ref to f1
    // args are in r1 and r2
    // r3 will contain the return value.
    builder.createLoadFuncInstr(0, 1);
    builder.createStoreSmallIntInstr(1, r1);
    builder.createStoreSmallIntInstr(2, r2);
    builder.createCallInstr(0, 3);
    builder.createRetVoidInstr();
  }
  // f1 multiplies its parameters by two and returns their sum.
  BCFunction& f1 = theModule.createFunction();
  ASSERT_EQ(f1.getID(), 1u);
  {
    BCBuilder builder = f1.createBCBuilder();
    builder.createAddIntInstr(0, 0, 0);
    builder.createAddIntInstr(1, 1, 1);
    builder.createAddIntInstr(2, 0, 1);
    builder.createRetInstr(2);
  }
  // Test
  VM vm(theModule);
  vm.call(f0);
  // Helper to get a register's value as a FoxInt
  auto getReg = [&](std::size_t idx) {
    return vm.getRegisterStack()[idx].intVal;
  };
  EXPECT_EQ(getReg(1), r1*2)            << "incorrect value for r1";
  EXPECT_EQ(getReg(2), r2*2)            << "incorrect value for r2";
  EXPECT_EQ(getReg(3), (r1*2) + (r2*2)) << "incorrect return value";
}

TEST_F(VMTest, stringCreation) {
  VM vm(theModule);
  static constexpr char helloWorld[] = "Hello, World!";
  
  {
    theModule.addStringConstant("Hello, World!");
    StringObject* string = vm.newStringObjectFromK(0);
    ASSERT_NE(string, nullptr);
    ASSERT_EQ(string->str(), helloWorld);
  }

  {
    StringObject* string = vm.newStringObject(helloWorld);
    ASSERT_NE(string, nullptr);
    ASSERT_EQ(string->str(), helloWorld);
  }

  {
    StringObject* string = vm.newStringObject();
    ASSERT_NE(string, nullptr);
    ASSERT_EQ(string->str(), "");
  }
}
