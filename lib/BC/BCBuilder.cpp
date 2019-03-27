//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BCBuilder.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BC/BCBuilder.hpp"
#include "Fox/BC/Instruction.hpp"

using namespace fox;

//----------------------------------------------------------------------------//
// BCModuleBuilder: Macro-generated methods
//----------------------------------------------------------------------------//

#define SIMPLE_INSTR(ID)                                                       \
  BCModule::instr_iterator BCModuleBuilder::create##ID##Instr() {              \
    return getModule().addInstr(Instruction(Opcode::ID));                      \
  }

#define TERNARY_INSTR(ID, I1, T1, I2, T2, I3, T3)                              \
  BCModule::instr_iterator                                                     \
  BCModuleBuilder::create##ID##Instr(T1 I1, T2 I2, T3 I3) {                    \
    Instruction instr(Opcode::ID);                                             \
    instr.ID.I1 = I1;                                                          \
    instr.ID.I2 = I2;                                                          \
    instr.ID.I3 = I3;                                                          \
    return getModule().addInstr(instr);                                        \
  }

#define BINARY_INSTR(ID, I1, T1, I2, T2)                                       \
  BCModule::instr_iterator                                                     \
  BCModuleBuilder::create##ID##Instr(T1 I1, T2 I2) {                           \
    Instruction instr(Opcode::ID);                                             \
    instr.ID.I1 = I1;                                                          \
    instr.ID.I2 = I2;                                                          \
    return getModule().addInstr(instr);                                        \
  }

#define UNARY_INSTR(ID, I1, T1)                                                \
  BCModule::instr_iterator BCModuleBuilder::create##ID##Instr(T1 I1) {         \
    Instruction instr(Opcode::ID);                                             \
    instr.ID.I1 = I1;                                                          \
    return getModule().addInstr(instr);                                        \
  }

#include "Fox/BC/Instruction.def"

//----------------------------------------------------------------------------//
// BCModuleBuilder
//----------------------------------------------------------------------------//

BCModuleBuilder::BCModuleBuilder() = default;
BCModuleBuilder::~BCModuleBuilder() = default;

BCModuleBuilder::instr_iterator BCModuleBuilder::getLastInstr() {
  return getModule().instrs_back();
}

void BCModuleBuilder::truncate_instrs(instr_iterator beg) {
  getInstrBuffer().erase(beg.toIBiterator(), getInstrBuffer().end());
}

bool BCModuleBuilder::isLastInstr(instr_iterator it) const {
  // TODO: Once I have a const version of instrs_back, remove
  // the const_cast.
  return (it == const_cast<BCModuleBuilder*>(this)->getLastInstr());
}

void BCModuleBuilder::popInstr() {
  getModule().getInstructionBuffer().pop_back();
}

std::unique_ptr<BCModule> BCModuleBuilder::takeModule() {
  return std::move(bcModule_);
}

BCModule& BCModuleBuilder::getModule() {
  // Lazily create a new module if needed.
  if(!bcModule_) bcModule_ = std::make_unique<BCModule>();
  return *bcModule_;
}

const BCModule& BCModuleBuilder::getModule() const {
  return const_cast<BCModuleBuilder*>(this)->getModule();
}

InstructionBuffer& BCModuleBuilder::getInstrBuffer() {
  return getModule().getInstructionBuffer();
}

const InstructionBuffer& BCModuleBuilder::getInstrBuffer() const {
  return getModule().getInstructionBuffer();
}
