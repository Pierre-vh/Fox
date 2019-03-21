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

#define TERNARY_INSTR(ID, T1, T2, T3)                                          \
  BCModule::instr_iterator                                                     \
  BCModuleBuilder::create##ID##Instr(T1 arg0, T2 arg1, T3 arg2) {              \
    Instruction instr(Opcode::ID);                                             \
    instr.ID.arg0 = arg0;                                                      \
    instr.ID.arg1 = arg1;                                                      \
    instr.ID.arg2 = arg2;                                                      \
    return getModule().addInstr(instr);                                        \
  }

#define BINARY_INSTR(ID, T1, T2)                                               \
  BCModule::instr_iterator                                                     \
  BCModuleBuilder::create##ID##Instr(T1 arg0, T2 arg1) {                       \
    Instruction instr(Opcode::ID);                                             \
    instr.ID.arg0 = arg0;                                                      \
    instr.ID.arg1 = arg1;                                                      \
    return getModule().addInstr(instr);                                        \
  }

#define UNARY_INSTR(ID, T1)                                                    \
  BCModule::instr_iterator BCModuleBuilder::create##ID##Instr(T1 arg) {        \
    Instruction instr(Opcode::ID);                                             \
    instr.ID.arg = arg;                                                        \
    return getModule().addInstr(instr);                                        \
  }

#include "Fox/BC/Instruction.def"

//----------------------------------------------------------------------------//
// BCModuleBuilder
//----------------------------------------------------------------------------//

BCModuleBuilder::BCModuleBuilder() = default;
BCModuleBuilder::~BCModuleBuilder() = default;

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