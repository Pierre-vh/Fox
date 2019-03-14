//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : VM.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/VM/VM.hpp"
#include "Fox/VM/Instructions.hpp"
#include "Fox/Common/Errors.hpp"
#include <cmath>

using namespace fox;

void VM::load(ArrayRef<Instruction> instrs) {
  assert((instrs.back().opcode == Opcode::Break)
    && "The last instruction of the program is not a 'break' instruction");
  program_ = instrs;
  programCounter_ = 0;
}

void VM::run() {
  Instruction instr;
  do {
    // Decode the current instruction
    instr = program_[programCounter_];
    #define TRIVIAL_TAC_BINOP_IMPL(ID, TYPE, OP)\
      setReg(instr.ID.arg0,\
      getReg<TYPE>(instr.ID.arg1) OP getReg<TYPE>(instr.ID.arg2))
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
        setReg(instr.StoreSmallInt.arg0, instr.StoreSmallInt.arg1);
        continue;
      case Opcode::AddInt: 
        TRIVIAL_TAC_BINOP_IMPL(AddInt, FoxInt, +);
        continue;
      case Opcode::AddDouble:
        // AddDouble A B C: A = B + C with B and C interpreted as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(AddDouble, FoxDouble, +);
        continue;
      case Opcode::SubInt:
        // SubInt A B C: A = B - C with B and C interpreted as FoxInts.
        TRIVIAL_TAC_BINOP_IMPL(SubInt, FoxInt, -);
        continue;
      case Opcode::SubDouble:
        // SubDouble A B C: A = B - C with B and C interpreted as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(SubDouble, FoxDouble, -);
        continue;
      case Opcode::MulInt:
        // DivInt A B C: A = B * C with B and C interpreted as FoxInts.
        TRIVIAL_TAC_BINOP_IMPL(MulInt, FoxInt, *);
        continue;
      case Opcode::MulDouble:
        // SubDouble A B C: A = B * C with B and C interpreted as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(MulDouble, FoxDouble, *);
        continue;
      case Opcode::DivInt:
        // DivInt A B C: A = B / C with B and C interpreted as FoxInts.
        // TO-DO: Handle this better
        assert(getReg<FoxInt>(instr.DivInt.arg2) && "division by zero");
        TRIVIAL_TAC_BINOP_IMPL(DivInt, FoxInt, /);
        continue;
      case Opcode::DivDouble:
        // SubDouble A B C: A = B / C with B and C interpreted as FoxDoubles.
        // TO-DO: Handle this better
        assert(getReg<FoxDouble>(instr.DivDouble.arg2) && "division by zero");
        TRIVIAL_TAC_BINOP_IMPL(DivDouble, FoxDouble, /);
        continue;
      case Opcode::ModInt:
        // ModInt A B C: A = B % C with B and C interpreted as FoxInts.
        // TO-DO: Handle this better
        assert(getReg<FoxInt>(instr.ModInt.arg2) && "Modulo by zero");
        TRIVIAL_TAC_BINOP_IMPL(ModInt, FoxInt, %);
        continue;
      case Opcode::ModDouble:
        // ModDouble A B C: A = B % C with B and C interpreted as FoxDoubles.
        // TO-DO: Handle this better
        assert(getReg<FoxDouble>(instr.ModDouble.arg2) && "Modulo by zero");
        setReg(instr.ModDouble.arg0, static_cast<FoxDouble>(
          std::fmod(
            getReg<FoxDouble>(instr.ModDouble.arg1), 
            getReg<FoxDouble>(instr.ModDouble.arg2)
          )
        ));
        continue;
      case Opcode::PowInt:
        // PowInt ModInt A B C: A = B pow C with B and C interpreted as FoxInts.
        setReg(instr.PowInt.arg0, static_cast<FoxInt>(
          std::pow(
            getReg<FoxInt>(instr.PowInt.arg1), 
            getReg<FoxInt>(instr.PowInt.arg2)
          )
        ));
        continue;
      case Opcode::PowDouble:
        // PowDouble A B C: A = B pow C with B and C interpreted as FoxDoubles.
        setReg(instr.PowDouble.arg0, static_cast<FoxDouble>(
          std::pow(
            getReg<FoxDouble>(instr.PowDouble.arg1), 
            getReg<FoxDouble>(instr.PowDouble.arg2)
          )
        ));
        continue;
      case Opcode::EqInt:
        // EqInt A B C: A = (B == C) with B and C interpreted as FoxInts.
        TRIVIAL_TAC_BINOP_IMPL(EqInt, FoxInt, ==);
        continue;
      case Opcode::LEInt:
        // LEInt A B C: A = (B <= C) with B and C interpreted as FoxInts.
        TRIVIAL_TAC_BINOP_IMPL(LEInt, FoxInt, <=);
        continue;
      case Opcode::LTInt:
        // LTInt A B C: A = (B < C) with B and C interpreted as FoxInts.
        TRIVIAL_TAC_BINOP_IMPL(LTInt, FoxInt, <);
        continue;
      case Opcode::EqDouble:
        // EqDouble A B C: A = (B == C) with B and C interpreted as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(EqDouble, FoxDouble, ==);
        continue;
      case Opcode::LEDouble:
        // LEDouble A B C: A = (B <= C) with B and C interpreted as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(LEDouble, FoxDouble, <=);
        continue;
      case Opcode::LTDouble:
        // LTDouble A B C: A = (B < C) with B and C interpreted as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(LTDouble, FoxDouble, <);
        continue;
      case Opcode::GEDouble:
        // GEDouble A B C: A = (B >= C) with B and C interpreted as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(GEDouble, FoxDouble, >=);
        continue;
      case Opcode::GTDouble:
        // GTDouble A B C: A = (B > C) with B and C interpreted as FoxDoubles.
        TRIVIAL_TAC_BINOP_IMPL(GTDouble, FoxDouble, >);
        continue;
      case Opcode::LOr:
        // LOr A B C: A = (B || C). B and C are raw register values.
        setReg(instr.LOr.arg0, 
              (getReg(instr.LOr.arg1) || getReg(instr.LOr.arg2)));
        continue;
      case Opcode::LAnd:
        // LAnd A B C: A = (B && C). B and C are raw register values.
        setReg(instr.LAnd.arg0, 
              (getReg(instr.LAnd.arg1) && getReg(instr.LAnd.arg2)));
        continue;
      case Opcode::LNot:
        // LNot A B: A = !B
        setReg(instr.LNot.arg0, !getReg(instr.LNot.arg1));
        continue;
      case Opcode::CondJump:
        // CondJump A D: Add D (as a signed int) to programCounter if A != 0
        if(getReg(instr.CondJump.arg0) != 0)
          programCounter_ += instr.CondJump.arg1;
        continue;
      case Opcode::Jump:
        // Jump x: Add x to programCounter
        programCounter_ += instr.Jump.arg;
        continue;
      case Opcode::IntToDouble:
        // IntToDouble r1 r2 : r1 = r2 as int with r2 interpreted as a FoxInt
        setReg(instr.IntToDouble.arg0, 
               FoxDouble(getReg<FoxInt>(instr.IntToDouble.arg1)));
        continue;
      case Opcode::DoubleToInt:
        // IntToDouble r1 r2 : r1 = r2 as double with r2 interpreted as a FoxDouble
        setReg(instr.DoubleToInt.arg0, 
               FoxInt(getReg<FoxDouble>(instr.DoubleToInt.arg1)));
        continue;
      default:
        fox_unreachable("illegal or unimplemented instruction found");
    }
    #undef TRIVIAL_TAC_BINOP_IMPL
    // Infinite loop where we increment the program counter
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
