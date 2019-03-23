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
  InstructionBuffer& program = bcModule_.getInstructionBuffer();
  do {
    // Fetch the current instruction
    instr = program[programCounter_];
    // Macros uset to implement repetitive operations
    #define TRIVIAL_TAC_BINOP_IMPL(ID, TYPE, OP)\
      setReg(instr.ID.dest,\
      getReg<TYPE>(instr.ID.lhs) OP getReg<TYPE>(instr.ID.rhs))
    // TODO: Rewrite the docs of each operands withou "A B C D"s
    switch (instr.opcode) {
      case Opcode::NoOp: 
        // NoOp: no-op: do nothing.
        continue;
      case Opcode::Break:
        // Break: stop the execution of the program.
        return;
      case Opcode::StoreSmallInt:
        // StoreSmallInt A B: Stores a small signed 16 bits integer (B) in r[A]

        // Note: D must be converted to a signed value before being placed
        // in r[A]
        setReg(instr.StoreSmallInt.dest, instr.StoreSmallInt.value);
        continue;
      case Opcode::AddInt: 
        TRIVIAL_TAC_BINOP_IMPL(AddInt, FoxInt, +);
        continue;
      case Opcode::AddDouble:
        // AddDouble A B C: A = B + C with B and C treated as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(AddDouble, FoxDouble, +);
        continue;
      case Opcode::SubInt:
        // SubInt A B C: A = B - C with B and C treated as FoxInts.
        TRIVIAL_TAC_BINOP_IMPL(SubInt, FoxInt, -);
        continue;
      case Opcode::SubDouble:
        // SubDouble A B C: A = B - C with B and C treated as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(SubDouble, FoxDouble, -);
        continue;
      case Opcode::MulInt:
        // DivInt A B C: A = B * C with B and C treated as FoxInts.
        TRIVIAL_TAC_BINOP_IMPL(MulInt, FoxInt, *);
        continue;
      case Opcode::MulDouble:
        // SubDouble A B C: A = B * C with B and C treated as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(MulDouble, FoxDouble, *);
        continue;
      case Opcode::DivInt:
        // DivInt A B C: A = B / C with B and C treated as FoxInts.
        // TO-DO: Handle this better
        assert(getReg<FoxInt>(instr.DivInt.rhs) && "division by zero");
        TRIVIAL_TAC_BINOP_IMPL(DivInt, FoxInt, /);
        continue;
      case Opcode::DivDouble:
        // SubDouble A B C: A = B / C with B and C treated as FoxDoubles.
        // TO-DO: Handle this better
        assert(getReg<FoxDouble>(instr.DivDouble.rhs) && "division by zero");
        TRIVIAL_TAC_BINOP_IMPL(DivDouble, FoxDouble, /);
        continue;
      case Opcode::ModInt:
        // ModInt A B C: A = B % C with B and C treated as FoxInts.
        // TO-DO: Handle this better
        assert(getReg<FoxInt>(instr.ModInt.rhs) && "Modulo by zero");
        TRIVIAL_TAC_BINOP_IMPL(ModInt, FoxInt, %);
        continue;
      case Opcode::ModDouble:
        // ModDouble A B C: A = B % C with B and C treated as FoxDoubles.
        // TO-DO: Handle this better
        assert(getReg<FoxDouble>(instr.ModDouble.rhs) && "Modulo by zero");
        setReg(instr.ModDouble.dest, static_cast<FoxDouble>(
          std::fmod(
            getReg<FoxDouble>(instr.ModDouble.lhs), 
            getReg<FoxDouble>(instr.ModDouble.rhs)
          )
        ));
        continue;
      case Opcode::PowInt:
        // PowInt ModInt A B C: A = B pow C with B and C treated as FoxInts.
        setReg(instr.PowInt.dest, static_cast<FoxInt>(
          std::pow(
            getReg<FoxInt>(instr.PowInt.lhs), 
            getReg<FoxInt>(instr.PowInt.rhs)
          )
        ));
        continue;
      case Opcode::PowDouble:
        // PowDouble A B C: A = B pow C, with B and C treated as FoxDoubles.
        setReg(instr.PowDouble.dest, static_cast<FoxDouble>(
          std::pow(
            getReg<FoxDouble>(instr.PowDouble.lhs), 
            getReg<FoxDouble>(instr.PowDouble.rhs)
          )
        ));
        continue;
      case Opcode::NegInt:
        // NegInt A B : A = -B, with B = FoxInt
        setReg(instr.NegInt.dest, -getReg<FoxInt>(instr.NegInt.src));
        continue;
      case Opcode::NegDouble:
        // NegDouble A B : A = -B, with B = FoxDouble
        setReg(instr.NegInt.dest, -getReg<FoxDouble>(instr.NegInt.src));
        continue;
      case Opcode::EqInt:
        // EqInt A B C: A = (B == C) with B and C treated as FoxInts.
        TRIVIAL_TAC_BINOP_IMPL(EqInt, FoxInt, ==);
        continue;
      case Opcode::LEInt:
        // LEInt A B C: A = (B <= C) with B and C treated as FoxInts.
        TRIVIAL_TAC_BINOP_IMPL(LEInt, FoxInt, <=);
        continue;
      case Opcode::LTInt:
        // LTInt A B C: A = (B < C) with B and C treated as FoxInts.
        TRIVIAL_TAC_BINOP_IMPL(LTInt, FoxInt, <);
        continue;
      case Opcode::EqDouble:
        // EqDouble A B C: A = (B == C) with B and C treated as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(EqDouble, FoxDouble, ==);
        continue;
      case Opcode::LEDouble:
        // LEDouble A B C: A = (B <= C) with B and C treated as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(LEDouble, FoxDouble, <=);
        continue;
      case Opcode::LTDouble:
        // LTDouble A B C: A = (B < C) with B and C treated as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(LTDouble, FoxDouble, <);
        continue;
      case Opcode::GEDouble:
        // GEDouble A B C: A = (B >= C) with B and C treated as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(GEDouble, FoxDouble, >=);
        continue;
      case Opcode::GTDouble:
        // GTDouble A B C: A = (B > C) with B and C treated as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(GTDouble, FoxDouble, >);
        continue;
      case Opcode::LOr:
        // LOr A B C: A = (B || C). B and C are raw register values.
        setReg(instr.LOr.dest, 
              (getReg(instr.LOr.lhs) || getReg(instr.LOr.rhs)));
        continue;
      case Opcode::LAnd:
        // LAnd A B C: A = (B && C). B and C are raw register values.
        setReg(instr.LAnd.dest, 
              (getReg(instr.LAnd.lhs) && getReg(instr.LAnd.rhs)));
        continue;
      case Opcode::LNot:
        // LNot A B: A = !B
        setReg(instr.LNot.dest, !getReg(instr.LNot.src));
        continue;
      case Opcode::CondJump:
        // CondJump A D: Add D (as a signed int) to programCounter if A != 0
        if(getReg(instr.CondJump.condReg) != 0)
          programCounter_ += instr.CondJump.offset;
        continue;
      case Opcode::Jump:
        // Jump x: Add x to programCounter
        programCounter_ += instr.Jump.offset;
        continue;
      case Opcode::IntToDouble:
        // IntToDouble r1 r2 : r1 = r2 as int with r2 treated as a FoxInt
        setReg(instr.IntToDouble.dest, 
               FoxDouble(getReg<FoxInt>(instr.IntToDouble.src)));
        continue;
      case Opcode::DoubleToInt:
        // IntToDouble r1 r2 : r1 = r2 as double with r2 treated as a FoxDouble
        setReg(instr.DoubleToInt.dest, 
               FoxInt(getReg<FoxDouble>(instr.DoubleToInt.src)));
        continue;
      case Opcode::Copy:
        // Copy r0 r1 : r1 = r2 (r2 copied in r1, performed on raw registers)
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
