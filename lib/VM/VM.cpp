//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : VM.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/VM/VM.hpp"
#include "Fox/BC/Instruction.hpp"
#include "Fox/BC/BCModule.hpp"
#include "Fox/Common/Errors.hpp"
#include <cmath>

using namespace fox;

VM::VM(BCModule& vmModule) : bcModule_(vmModule) {}

void VM::run() {
  Instruction instr;
  InstructionBuffer& program = bcModule_.getInstructions();
  do {
    // Fetch the current instruction
    instr = program[programCounter_];
    // Macros used to implement repetitive operations
    #define TRIVIAL_TAC_BINOP_IMPL(ID, TYPE, OP)\
      setReg(instr.ID.dest,\
      getReg<TYPE>(instr.ID.lhs) OP getReg<TYPE>(instr.ID.rhs))
    switch (instr.opcode) {
      case Opcode::NoOp: 
        // NoOp: no-op: do nothing.
        continue;
      case Opcode::Break:
        // Break: stop the execution of the program.
        return;
      case Opcode::StoreSmallInt:
        // StoreSmallInt dest value: Stores value in dest (value: int16)
        setReg(instr.StoreSmallInt.dest, instr.StoreSmallInt.value);
        continue;
      case Opcode::AddInt: 
        // AddInt dest lhs C: dest = lhs + rhs (FoxInts)
        TRIVIAL_TAC_BINOP_IMPL(AddInt, FoxInt, +);
        continue;
      case Opcode::AddDouble:
        // AddDouble dest lhs C: dest = lhs + rhs (FoxDoubles)
        TRIVIAL_TAC_BINOP_IMPL(AddDouble, FoxDouble, +);
        continue;
      case Opcode::SubInt:
        // SubInt dest lhs C: dest = lhs - rhs (FoxInts)
        TRIVIAL_TAC_BINOP_IMPL(SubInt, FoxInt, -);
        continue;
      case Opcode::SubDouble:
        // SubDouble dest lhs C: dest = lhs - rhs (FoxDoubles)
        TRIVIAL_TAC_BINOP_IMPL(SubDouble, FoxDouble, -);
        continue;
      case Opcode::MulInt:
        // DivInt dest lhs C: dest = lhs * rhs (FoxInts)
        TRIVIAL_TAC_BINOP_IMPL(MulInt, FoxInt, *);
        continue;
      case Opcode::MulDouble:
        // SubDouble dest lhs C: dest = lhs * rhs (FoxDoubles)
        TRIVIAL_TAC_BINOP_IMPL(MulDouble, FoxDouble, *);
        continue;
      case Opcode::DivInt:
        // DivInt dest lhs C: dest = lhs / rhs (FoxInts)
        // TO-DO: Handle division by zero with something else than an assert
        assert(getReg<FoxInt>(instr.DivInt.rhs) && "division by zero");
        TRIVIAL_TAC_BINOP_IMPL(DivInt, FoxInt, /);
        continue;
      case Opcode::DivDouble:
        // SubDouble dest lhs C: dest = lhs / rhs (FoxDoubles)
        // TO-DO: Handle division by zero with something else than an assert
        assert(getReg<FoxDouble>(instr.DivDouble.rhs) && "division by zero");
        TRIVIAL_TAC_BINOP_IMPL(DivDouble, FoxDouble, /);
        continue;
      case Opcode::ModInt:
        // ModInt dest lhs rhs: dest = lhs % rhs (FoxInts)
        // TO-DO: Handle modulo by zero with something else than an assert
        assert(getReg<FoxInt>(instr.ModInt.rhs) && "Modulo by zero");
        TRIVIAL_TAC_BINOP_IMPL(ModInt, FoxInt, %);
        continue;
      case Opcode::ModDouble:
        // ModDouble dest lhs rhs: dest = lhs % rhs (FoxDoubles)
        // TO-DO: Handle modulo by zero with something else than an assert
        assert(getReg<FoxDouble>(instr.ModDouble.rhs) && "Modulo by zero");
        setReg(instr.ModDouble.dest, static_cast<FoxDouble>(
          std::fmod(
            getReg<FoxDouble>(instr.ModDouble.lhs), 
            getReg<FoxDouble>(instr.ModDouble.rhs)
          )
        ));
        continue;
      case Opcode::PowInt:
        // PowInt dest lhs rhs: dest = pow(lhs, rhs) (FoxInts)
        setReg(instr.PowInt.dest, static_cast<FoxInt>(
          std::pow(
            getReg<FoxInt>(instr.PowInt.lhs), 
            getReg<FoxInt>(instr.PowInt.rhs)
          )
        ));
        continue;
      case Opcode::PowDouble:
        // PowDouble dest lhs rhs: dest = pow(lhs, rhs) (FoxDoubles)
        setReg(instr.PowDouble.dest, static_cast<FoxDouble>(
          std::pow(
            getReg<FoxDouble>(instr.PowDouble.lhs), 
            getReg<FoxDouble>(instr.PowDouble.rhs)
          )
        ));
        continue;
      case Opcode::NegInt:
        // NegInt dest src : dest = -src (FoxInts)
        setReg(instr.NegInt.dest, -getReg<FoxInt>(instr.NegInt.src));
        continue;
      case Opcode::NegDouble:
        // NegDouble dest src : dest = -src (FoxDoubles)
        setReg(instr.NegInt.dest, -getReg<FoxDouble>(instr.NegInt.src));
        continue;
      case Opcode::EqInt:
        // EqInt dest lhs rhs: dest = (lhs == rhs) 
        //          (lhs/rhs: FoxInts, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(EqInt, FoxInt, ==);
        continue;
      case Opcode::LEInt:
        // LEInt dest lhs rhs: dest = (lhs <= rhs) 
        //          (lhs/rhs: FoxInts, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(LEInt, FoxInt, <=);
        continue;
      case Opcode::LTInt:
        // LTInt dest lhs rhs: dest = (lhs < rhs) 
        //          (lhs/rhs: FoxInts, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(LTInt, FoxInt, <);
        continue;
      case Opcode::EqDouble:
        // EqDouble dest lhs rhs: dest = (lhs == rhs) 
        //          (lhs/rhs: FoxDoubles, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(EqDouble, FoxDouble, ==);
        continue;
      case Opcode::LEDouble:
        // LEDouble dest lhs rhs: dest = (lhs <= rhs) 
        //          (lhs/rhs: FoxDoubles, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(LEDouble, FoxDouble, <=);
        continue;
      case Opcode::LTDouble:
        // LTDouble dest lhs rhs: dest = (lhs < rhs) 
        //          (lhs/rhs: FoxDoubles, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(LTDouble, FoxDouble, <);
        continue;
      case Opcode::GEDouble:
        // GEDouble dest lhs rhs: dest = (lhs >= rhs) 
        //          (lhs/rhs: FoxDoubles, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(GEDouble, FoxDouble, >=);
        continue;
      case Opcode::GTDouble:
        // GTDouble dest lhs rhs: dest = (lhs > rhs) 
        //          (lhs/rhs: FoxDoubles, dest: raw)
        TRIVIAL_TAC_BINOP_IMPL(GTDouble, FoxDouble, >);
        continue;
      case Opcode::LOr:
        // LOr dest lhs rhs: dest = (lhs || rhs) (raw registers)
        setReg(instr.LOr.dest, 
              (getReg(instr.LOr.lhs) || getReg(instr.LOr.rhs)));
        continue;
      case Opcode::LAnd:
        // LAnd dest lhs rhs: dest = (lhs && rhs) (raw registers)
        setReg(instr.LAnd.dest, 
              (getReg(instr.LAnd.lhs) && getReg(instr.LAnd.rhs)));
        continue;
      case Opcode::LNot:
        // LNot dest src: dest = !src
        setReg(instr.LNot.dest, !getReg(instr.LNot.src));
        continue;
      case Opcode::CondJump:
        // CondJump condReg offset : Add offset (int16) to programCounter 
        //    if condReg != 0
        if(getReg(instr.CondJump.condReg) != 0)
          programCounter_ += instr.CondJump.offset;
        continue;
      case Opcode::Jump:
        // Jump offset: Add offset (int16) to programCounter
        programCounter_ += instr.Jump.offset;
        continue;
      case Opcode::IntToDouble:
        // IntToDouble dest src: dest = (src as FoxDouble) (src: FoxInt)
        setReg(instr.IntToDouble.dest, 
               FoxDouble(getReg<FoxInt>(instr.IntToDouble.src)));
        continue;
      case Opcode::DoubleToInt:
        // DoubleToInt dest src: dest = (src as FoxInt) (src: FoxDouble)
        setReg(instr.DoubleToInt.dest, 
               FoxInt(getReg<FoxDouble>(instr.DoubleToInt.src)));
        continue;
      case Opcode::Copy:
        // Copy dest src : dest = src
        setReg(instr.Copy.dest, getReg(instr.Copy.src));
        continue;
      default:
        fox_unreachable("illegal or unimplemented instruction found");
    }
    #undef TRIVIAL_TAC_BINOP_IMPL
  } while(++programCounter_);
}

std::size_t VM::getPC() const {
  return programCounter_;
}

ArrayRef<std::uint64_t> VM::getRegisterStack() const {
  return regStack_;
}

MutableArrayRef<std::uint64_t> VM::getRegisterStack() {
  return regStack_;
}
