//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : InstructionBuilder.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/VM/InstructionBuilder.hpp"
#include "Fox/VM/Instructions.hpp"
#include "Fox/VM/VMModule.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// InstructionBuilder: Macro-generated methods
//----------------------------------------------------------------------------//

#define SIMPLE_INSTR(ID)\
  InstructionBuilder& InstructionBuilder::create##ID##Instr() {\
    Instruction instr(Opcode::ID);                \
    pushInstr(instr);                             \
    return *this;                                 \
  }

#define TERNARY_INSTR(ID, T1, T2, T3)\
  InstructionBuilder& InstructionBuilder::\
  create##ID##Instr(T1 arg0, T2 arg1, T3 arg2) {  \
    Instruction instr(Opcode::ID);                \
    instr.ID.arg0 = arg0;                         \
    instr.ID.arg1 = arg1;                         \
    instr.ID.arg2 = arg2;                         \
    pushInstr(instr);                             \
    return *this;                                 \
  }

#define BINARY_INSTR(ID, T1, T2)\
  InstructionBuilder&\
  InstructionBuilder::create##ID##Instr(T1 arg0, T2 arg1) {\
    Instruction instr(Opcode::ID);                \
    instr.ID.arg0 = arg0;                         \
    instr.ID.arg1 = arg1;                         \
    pushInstr(instr);                             \
    return *this;                                 \
  }

#define UNARY_INSTR(ID, T1)\
  InstructionBuilder&\
  InstructionBuilder::create##ID##Instr(T1 arg) { \
    Instruction instr(Opcode::ID);                \
    instr.ID.arg = arg;                           \
    pushInstr(instr);                             \
    return *this;                                 \
  }

#include "Fox/VM/Instructions.def"

//----------------------------------------------------------------------------//
// InstructionBuilder
//----------------------------------------------------------------------------//

InstructionBuilder::InstructionBuilder() = default;
fox::InstructionBuilder::~InstructionBuilder() = default;

Instruction InstructionBuilder::getLastInstr() const {
  return getModule().getInstructionBuffer().back();
}

std::unique_ptr<VMModule> InstructionBuilder::takeModule() {
  return std::move(vmModule_);
}

VMModule& InstructionBuilder::getModule() {
  // Lazily create a new module if needed.
  if(!vmModule_) vmModule_ = std::make_unique<VMModule>();
  return *vmModule_;
}

const VMModule& InstructionBuilder::getModule() const {
  return const_cast<InstructionBuilder*>(this)->getModule();
}

void InstructionBuilder::pushInstr(Instruction instr) {
  getModule().getInstructionBuffer().push_back(instr);
}
