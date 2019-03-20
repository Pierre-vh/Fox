//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BCBuilder.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BC/BCBuilder.hpp"
#include "Fox/BC/Instructions.hpp"
#include "Fox/BC/BCModule.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// BCModuleBuilder: Macro-generated methods
//----------------------------------------------------------------------------//

#define SIMPLE_INSTR(ID)\
  BCModuleBuilder& BCModuleBuilder::create##ID##Instr() {\
    Instruction instr(Opcode::ID);                \
    pushInstr(instr);                             \
    return *this;                                 \
  }

#define TERNARY_INSTR(ID, T1, T2, T3)\
  BCModuleBuilder& BCModuleBuilder::\
  create##ID##Instr(T1 arg0, T2 arg1, T3 arg2) {  \
    Instruction instr(Opcode::ID);                \
    instr.ID.arg0 = arg0;                         \
    instr.ID.arg1 = arg1;                         \
    instr.ID.arg2 = arg2;                         \
    pushInstr(instr);                             \
    return *this;                                 \
  }

#define BINARY_INSTR(ID, T1, T2)\
  BCModuleBuilder&\
  BCModuleBuilder::create##ID##Instr(T1 arg0, T2 arg1) {\
    Instruction instr(Opcode::ID);                \
    instr.ID.arg0 = arg0;                         \
    instr.ID.arg1 = arg1;                         \
    pushInstr(instr);                             \
    return *this;                                 \
  }

#define UNARY_INSTR(ID, T1)\
  BCModuleBuilder&\
  BCModuleBuilder::create##ID##Instr(T1 arg) { \
    Instruction instr(Opcode::ID);                \
    instr.ID.arg = arg;                           \
    pushInstr(instr);                             \
    return *this;                                 \
  }

#include "Fox/BC/Instructions.def"

//----------------------------------------------------------------------------//
// BCModuleBuilder
//----------------------------------------------------------------------------//

BCModuleBuilder::BCModuleBuilder() = default;
fox::BCModuleBuilder::~BCModuleBuilder() = default;

Instruction BCModuleBuilder::getLastInstr() const {
  return getModule().getInstructionBuffer().back();
}

std::unique_ptr<BCModule> BCModuleBuilder::takeModule() {
  return std::move(vmModule_);
}

BCModule& BCModuleBuilder::getModule() {
  // Lazily create a new module if needed.
  if(!vmModule_) vmModule_ = std::make_unique<BCModule>();
  return *vmModule_;
}

const BCModule& BCModuleBuilder::getModule() const {
  return const_cast<BCModuleBuilder*>(this)->getModule();
}

void BCModuleBuilder::pushInstr(Instruction instr) {
  getModule().getInstructionBuffer().push_back(instr);
}
