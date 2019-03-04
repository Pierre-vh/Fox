//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : VM.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/VM/VM.hpp"
#include "Fox/VM/Opcode.hpp"
#include "Fox/Common/Errors.hpp"
#include <cmath>

using namespace fox;

// In many places of the VM, we assume that signed numbers are encoded using
// 2's complement. Assert that it's the case.
static_assert(-1 == 0xffffffff,
  "Can't compile the Fox VM because signed integers"
  " are not encoded using 2's complement");

namespace {
  inline Opcode getInstrOpcode(std::uint32_t instr) {
    return static_cast<Opcode>(instr & 0x000000FF);
  }
}

void VM::load(ArrayRef<std::uint32_t> instrs) {
  assert((getInstrOpcode(instrs.back()) == Opcode::Break)
    && "The last instruction of the program is not a 'break' instruction");
  program_ = instrs;
  programCounter_ = 0;
}

void VM::run() {
  std::uint32_t instr = 0;
  while (true) {
    // Decode the current instruction
    instr = program_[programCounter_++];
    #define opcode static_cast<Opcode>(instr & 0x000000FF)
    // Maybe theses could be renamed so they aren't one letter identifiers?
    #define a static_cast<std::uint8_t>((instr & 0x0000FF00) >> 8)
    #define b static_cast<std::uint8_t>((instr & 0x00FF0000) >> 16)
    #define c static_cast<std::uint8_t>((instr & 0xFF000000) >> 24)
    #define d static_cast<std::uint16_t>((instr & 0xFFFF0000) >> 16)
    switch (opcode) {
      case Opcode::NoOp: 
        // NoOp: no-op: do nothing.
        continue;
      case Opcode::Break:
        // Break: stop the execution of the program.
        return;
      case Opcode::StoreSmallInt:
        // StoreSmallInt A D: Stores a small signed 16 bits integer (D) in r[A]

        // Note: D must be converted to a signed value before being placed
        // in r[A]
        setReg(a, static_cast<std::int16_t>(d));
        continue;
      case Opcode::AddInt: 
        // AddInt A B C: A = B + C with B and C interpreted as FoxInts.
        setReg(a, getReg<FoxInt>(b) + getReg<FoxInt>(c));
        continue;
      case Opcode::AddDouble:
        // AddDouble A B C: A = B + C with B and C interpreted as FoxDoubles.
        setReg(a, getReg<FoxDouble>(b) + getReg<FoxDouble>(c));
        continue;
      case Opcode::SubInt:
        // SubInt A B C: A = B - C with B and C interpreted as FoxInts.
        setReg(a, getReg<FoxInt>(b) - getReg<FoxInt>(c));
        continue;
      case Opcode::SubDouble:
        // SubDouble A B C: A = B - C with B and C interpreted as FoxDoubles.
        setReg(a, getReg<FoxDouble>(b) - getReg<FoxDouble>(c));
        continue;
      case Opcode::MulInt:
        // DivInt A B C: A = B * C with B and C interpreted as FoxInts.
        setReg(a, getReg<FoxInt>(b) * getReg<FoxInt>(c));
        continue;
      case Opcode::MulDouble:
        // SubDouble A B C: A = B * C with B and C interpreted as FoxDoubles.
        setReg(a, getReg<FoxDouble>(b) * getReg<FoxDouble>(c));
        continue;
      case Opcode::DivInt:
        // DivInt A B C: A = B / C with B and C interpreted as FoxInts.
        // TO-DO: Handle this better
        assert(getReg<FoxInt>(c) && "division by zero");
        setReg(a, getReg<FoxInt>(b) / getReg<FoxInt>(c));
        continue;
      case Opcode::DivDouble:
        // SubDouble A B C: A = B / C with B and C interpreted as FoxDoubles.
        // TO-DO: Handle this better
        assert(getReg<FoxDouble>(c) && "division by zero");
        setReg(a, getReg<FoxDouble>(b) / getReg<FoxDouble>(c));
        continue;
      case Opcode::ModInt:
        // ModInt A B C: A = B % C with B and C interpreted as FoxInts.
        // TO-DO: Handle this better
        assert(getReg<FoxInt>(c) && "modulo by zero");
        setReg(a, getReg<FoxInt>(b) % getReg<FoxInt>(c));
        continue;
      case Opcode::ModDouble:
        // ModDouble A B C: A = B % C with B and C interpreted as FoxDoubles.
        // TO-DO: Handle this better
        assert(getReg<FoxDouble>(c) && "modulo by zero");
        setReg(a, static_cast<FoxDouble>(
          std::fmod(getReg<FoxDouble>(b), getReg<FoxDouble>(c))
        ));
        continue;
      case Opcode::PowInt:
        // PowInt ModInt A B C: A = B pow C with B and C interpreted as FoxInts.
        setReg(a, static_cast<FoxInt>(
          std::pow(getReg<FoxInt>(b), getReg<FoxInt>(c))
        ));
        continue;
      case Opcode::PowDouble:
        // PowDouble A B C: A = B pow C with B and C interpreted as FoxDoubles.
        setReg(a, static_cast<FoxDouble>(
          std::pow(getReg<FoxDouble>(b), getReg<FoxDouble>(c))
        ));
        continue;
      case Opcode::EqInt:
        // EqInt A B C: A = (B == C) with B and C interpreted as FoxInts.
        setReg(a, (getReg<FoxInt>(b) == getReg<FoxInt>(c)));
        continue;
      case Opcode::LEInt:
        // LEInt A B C: A = (B <= C) with B and C interpreted as FoxInts.
        setReg(a, (getReg<FoxInt>(b) <= getReg<FoxInt>(c)));
        continue;
      case Opcode::LTInt:
        // LTInt A B C: A = (B < C) with B and C interpreted as FoxInts.
        setReg(a, (getReg<FoxInt>(b) < getReg<FoxInt>(c)));
        continue;
      case Opcode::EqDouble:
        // EqDouble A B C: A = (B == C) with B and C interpreted as FoxDoubles.
        setReg(a, (getReg<FoxDouble>(b) == getReg<FoxDouble>(c)));
        continue;
      case Opcode::LEDouble:
        // LEDouble A B C: A = (B <= C) with B and C interpreted as FoxDoubles.
        setReg(a, (getReg<FoxDouble>(b) <= getReg<FoxDouble>(c)));
        continue;
      case Opcode::LTDouble:
        // LTDouble A B C: A = (B < C) with B and C interpreted as FoxDoubles.
        setReg(a, (getReg<FoxDouble>(b) < getReg<FoxDouble>(c)));
        continue;
      case Opcode::GEDouble:
        // GEDouble A B C: A = (B >= C) with B and C interpreted as FoxDoubles.
        setReg(a, (getReg<FoxDouble>(b) >= getReg<FoxDouble>(c)));
        continue;
      case Opcode::GTDouble:
        // GTDouble A B C: A = (B > C) with B and C interpreted as FoxDoubles.
        setReg(a, (getReg<FoxDouble>(b) > getReg<FoxDouble>(c)));
        continue;
      case Opcode::LOr:
        // LOr A B C: A = (B || C). B and C are raw register values.
        setReg(a, (getReg(b) || getReg(c)));
        continue;
      case Opcode::LAnd:
        // LAnd A B C: A = (B && C). B and C are raw register values.
        setReg(a, (getReg(b) && getReg(c)));
        continue;
      case Opcode::LNot:
        // LNot A B: A = !B
        setReg(a, !getReg(b));
        continue;
      default:
        fox_unreachable("illegal or unimplemented instruction found");
    }
    #undef opcode
    #undef a
    #undef b
    #undef c
    #undef d
  }
}

ArrayRef<std::uint64_t> VM::getRegisterStack() const {
  return regStack_;
}

MutableArrayRef<std::uint64_t> VM::getRegisterStack() {
  return regStack_;
}
