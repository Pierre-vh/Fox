//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : InstructionBuilder.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/VM/InstructionBuilder.hpp"
#include "Fox/VM/Instructions.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// InstructionBuilder: Macro-generated methods
//----------------------------------------------------------------------------//

#define SIMPLE_INSTR(ID)\
  InstructionBuilder& InstructionBuilder::create##ID##Instr() {\
    Instruction instr;                           \
    instr.opcode = Opcode::ID;                   \
    pushInstr(instr);                            \
    return *this;                                \
  }

#define TERNARY_INSTR(ID, T1, T2, T3)\
  InstructionBuilder& InstructionBuilder::\
  create##ID##Instr(T1 arg0, T2 arg1, T3 arg2) {\
    Instruction instr;                           \
    instr.opcode = Opcode::ID;                   \
    instr.ID.arg0 = arg0;                        \
    instr.ID.arg1 = arg1;                        \
    instr.ID.arg2 = arg2;                        \
    pushInstr(instr);                            \
    return *this;                                \
  }

#define BINARY_INSTR(ID, T1, T2)\
  InstructionBuilder&\
  InstructionBuilder::create##ID##Instr(T1 arg0, T2 arg1) {\
    Instruction instr;                           \
    instr.opcode = Opcode::ID;                   \
    instr.ID.arg0 = arg0;                        \
    instr.ID.arg1 = arg1;                        \
    pushInstr(instr);                            \
    return *this;                                \
  }

#define UNARY_INSTR(ID, T1)\
  InstructionBuilder&\
  InstructionBuilder::create##ID##Instr(T1 arg) {\
    Instruction instr;                           \
    instr.opcode = Opcode::ID;                   \
    instr.ID.arg = arg;                          \
    pushInstr(instr);                            \
    return *this;                                \
  }

#include "Fox/VM/Instructions.def"

//----------------------------------------------------------------------------//
// InstructionBuilder
//----------------------------------------------------------------------------//

void InstructionBuilder::reset() {
  if(hasBuffer())
    getBuffer().clear();
}

Instruction InstructionBuilder::getLastInstr() const {
  if(hasBuffer())
    return getBuffer().back();
  return Instruction();
}

ArrayRef<Instruction> InstructionBuilder::getInstrs() const {
  return getBuffer();
}

void InstructionBuilder::pushInstr(Instruction instr) {
  getBuffer().push_back(instr);
}

bool InstructionBuilder::hasBuffer() const {
  return (bool)instrBuffer_;
}

InstructionBuilder::Buffer& InstructionBuilder::getBuffer() {
  if(!instrBuffer_)
    instrBuffer_ = std::make_unique<Buffer>();
  return (*instrBuffer_);
}

const InstructionBuilder::Buffer& InstructionBuilder::getBuffer() const {
  return const_cast<InstructionBuilder*>(this)->getBuffer();
}
