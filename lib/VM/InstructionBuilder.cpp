//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : InstructionBuilder.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/VM/InstructionBuilder.hpp"
#include "Fox/VM/Opcode.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// InstructionBuilder: Macro-generated methods
//----------------------------------------------------------------------------//

#define SIMPLE_INSTR(ID)\
  InstructionBuilder& InstructionBuilder::create##ID##Instr()\
    { return createSimpleInstr(Opcode::ID); }

#define TERNARY_INSTR(ID)\
  InstructionBuilder& InstructionBuilder::\
  create##ID##Instr(std::uint8_t a, std::uint8_t b, std::uint8_t c) {\
    return createABCInstr(Opcode::ID, a, b, c);\
  }
#define SMALL_BINARY_INSTR(ID)\
  InstructionBuilder&\
  InstructionBuilder::create##ID##Instr(std::uint8_t a, std::uint8_t b) {\
    return createABCInstr(Opcode::ID, a, b, 0u);\
  }
#define BINARY_INSTR(ID)\
  InstructionBuilder&\
  InstructionBuilder::create##ID##Instr(std::uint8_t a, std::uint16_t d) {\
    return createADInstr(Opcode::ID, a, d);\
  }

#define UNARY_INSTR(ID)\
InstructionBuilder&\
InstructionBuilder::create##ID##Instr(std::uint32_t val) {\
  return createUnaryInstr(Opcode::ID, val);\
}

#define SIGNED_UNARY_INSTR(ID)\
InstructionBuilder&\
InstructionBuilder::create##ID##Instr(std::int32_t val) {\
  return createSignedUnaryInstr(Opcode::ID, val);\
}

#include "Fox/VM/Instructions.def"

//----------------------------------------------------------------------------//
// InstructionBuilder
//----------------------------------------------------------------------------//

void InstructionBuilder::reset() {
  instrsBuff_.clear();
}

std::uint32_t InstructionBuilder::getLastInstr() const {
  return instrsBuff_.back();
}

ArrayRef<std::uint32_t>InstructionBuilder::getInstrs() const {
  return instrsBuff_;
}

InstructionBuilder&
InstructionBuilder::createSimpleInstr(Opcode op) {  
  pushInstr(static_cast<std::uint8_t>(op));
  return *this;
}

InstructionBuilder& 
InstructionBuilder::createABCInstr(Opcode op, std::uint8_t a, 
                                  std::uint8_t b, std::uint8_t c) {
  std::uint32_t instr = 0;
  instr |= c;
  instr = (instr << 8) | b;
  instr = (instr << 8) | a;
  instr = (instr << 8)  | static_cast<std::uint8_t>(op);
  pushInstr(instr);
  return *this;
}

InstructionBuilder& 
InstructionBuilder::createADInstr(Opcode op, std::uint8_t a, std::uint16_t d) {
  std::uint32_t instr = 0;
  instr |= d;
  instr = (instr << 8)  | a;
  instr = (instr << 8)  | static_cast<std::uint8_t>(op);
  pushInstr(instr);
  return *this;
}

InstructionBuilder& 
InstructionBuilder::createSignedUnaryInstr(Opcode op, std::int32_t val) {
  assert((val >= -((2 << 23) - 1)) && (val <= ((2 << 23) - 1)) &&
    "Value is too small/large to fit in 24 bits!");
  return createUnaryInstr(op, val & 0x00FFFFFF);
}

InstructionBuilder& 
InstructionBuilder::createUnaryInstr(Opcode op, std::uint32_t val) {
  assert(((val & 0xFF000000) == 0) &&
    "Value is too small/large to fit in 24 bits!");
  std::uint32_t instr = 0;
  instr |= val;
  instr = (instr << 8) | static_cast<std::uint8_t>(op);
  pushInstr(instr);
  return *this;
}

void InstructionBuilder::pushInstr(std::uint32_t instr) {
  instrsBuff_.push_back(instr);
}
