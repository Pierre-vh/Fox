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

namespace {
  inline Opcode getInstrOpcode(std::uint32_t instr) {
    return static_cast<Opcode>(instr & 0x000000FF);
  }

  // TODO: Clean this "InstrData" mess up.

  // Small struct containing a few references to the current
  // instruction's data.
  struct InstrData {
    const Opcode* opcode = nullptr;
    const std::uint8_t* a = nullptr;
    const std::uint8_t* b = nullptr;
    const std::uint8_t* c = nullptr;
    const std::uint16_t* d = nullptr;
  };

  inline void decode(const std::uint32_t* instr, InstrData& to) {
    const std::uint8_t* ptr = reinterpret_cast<const std::uint8_t*>(instr);
    to.opcode = reinterpret_cast<const Opcode*>(ptr);
    to.a = ptr+1;
    to.b = ptr+2;
    to.c = ptr+3;
    to.d = reinterpret_cast<const std::uint16_t*>(ptr+2);
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
        // AddInt A B C: A = B + C where B and C are interpreted as FoxInts.
        setReg(a, getReg<FoxInt>(b) + getReg<FoxInt>(c));
        continue;
      case Opcode::AddDouble:
        // AddDouble A B C: A = B + C where B and C are interpreted as FoxDoubles.
        setReg(a, getReg<FoxDouble>(b) + getReg<FoxDouble>(c));
        continue;
      case Opcode::SubInt:
        // SubInt A B C: A = B - C where B and C are interpreted as FoxInts.
        setReg(a, getReg<FoxInt>(b) - getReg<FoxInt>(c));
        continue;
      case Opcode::SubDouble:
        // SubDouble A B C: A = B - C where B and C are interpreted as FoxDoubles.
        setReg(a, getReg<FoxDouble>(b) - getReg<FoxDouble>(c));
        continue;
      case Opcode::MulInt:
        // DivInt A B C: A = B * C where B and C are interpreted as FoxInts.
        setReg(a, getReg<FoxInt>(b) * getReg<FoxInt>(c));
        continue;
      case Opcode::MulDouble:
        // SubDouble A B C: A = B * C where B and C are interpreted as FoxDoubles.
        setReg(a, getReg<FoxDouble>(b) * getReg<FoxDouble>(c));
        continue;
      case Opcode::DivInt:
        // DivInt A B C: A = B / C where B and C are interpreted as FoxInts.
        // TO-DO: Handle this better
        assert(getReg<FoxInt>(c) && "division by zero");
        setReg(a, getReg<FoxInt>(b) / getReg<FoxInt>(c));
        continue;
      case Opcode::DivDouble:
        // SubDouble A B C: A = B / C where B and C are interpreted as FoxDoubles.
        // TO-DO: Handle this better
        assert(getReg<FoxDouble>(c) && "division by zero");
        setReg(a, getReg<FoxDouble>(b) / getReg<FoxDouble>(c));
        continue;
      case Opcode::ModInt:
        // ModInt A B C: A = B % C where B and C are interpreted as FoxInts.
        // TO-DO: Handle this better
        assert(getReg<FoxInt>(c) && "modulo by zero");
        setReg(a, getReg<FoxInt>(b) % getReg<FoxInt>(c));
        continue;
      case Opcode::ModDouble:
        // ModDouble A B C: A = B % C where B and C are interpreted as FoxDoubles.
        // TO-DO: Handle this better
        assert(getReg<FoxDouble>(c) && "modulo by zero");
        setReg(a, static_cast<FoxDouble>(
          std::fmod(getReg<FoxDouble>(b), getReg<FoxDouble>(c))
        ));
        continue;
      case Opcode::PowInt:
        // PowInt ModInt A B C: A = B pow C where B and C are interpreted as FoxInts.
        setReg(a, static_cast<FoxInt>(
          std::pow(getReg<FoxInt>(b), getReg<FoxInt>(c))
        ));
        continue;
      case Opcode::PowDouble:
        // PowDouble A B C: A = B pow C where B and C are interpreted as FoxDoubles.
        setReg(a, static_cast<FoxDouble>(
          std::pow(getReg<FoxDouble>(b), getReg<FoxDouble>(c))
        ));
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