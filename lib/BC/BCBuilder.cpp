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
// BCBuilder: Macro-generated methods
//----------------------------------------------------------------------------//

#define SIMPLE_INSTR(ID)                                                       \
  BCBuilder::StableInstrIter BCBuilder::create##ID##Instr() {                  \
    return insert(Instruction(Opcode::ID));                                    \
  }

#define TERNARY_INSTR(ID, I1, T1, I2, T2, I3, T3)                              \
  BCBuilder::StableInstrIter                                                   \
  BCBuilder::create##ID##Instr(T1 I1, T2 I2, T3 I3) {                          \
    Instruction instr(Opcode::ID);                                             \
    instr.ID.I1 = I1;                                                          \
    instr.ID.I2 = I2;                                                          \
    instr.ID.I3 = I3;                                                          \
    return insert(instr);                                                      \
  }

#define BINARY_INSTR(ID, I1, T1, I2, T2)                                       \
  BCBuilder::StableInstrIter BCBuilder::create##ID##Instr(T1 I1, T2 I2) {      \
    Instruction instr(Opcode::ID);                                             \
    instr.ID.I1 = I1;                                                          \
    instr.ID.I2 = I2;                                                          \
    return insert(instr);                                                      \
  }

#define UNARY_INSTR(ID, I1, T1)                                                \
  BCBuilder::StableInstrIter BCBuilder::create##ID##Instr(T1 I1) {             \
    Instruction instr(Opcode::ID);                                             \
    instr.ID.I1 = I1;                                                          \
    return insert(instr);                                                      \
  }

#include "Fox/BC/Instruction.def"

//----------------------------------------------------------------------------//
// BCBuilder
//----------------------------------------------------------------------------//

BCBuilder::BCBuilder(InstructionVector& vector, DebugInfo* debugInfo) 
  : vector(vector), debugInfo(debugInfo) {}

void BCBuilder::truncate_instrs(StableInstrIter beg) {
  vector.erase(beg.getContainerIterator(), vector.end());
}

LLVM_NODISCARD bool BCBuilder::empty() const {
  return vector.empty();
}

bool BCBuilder::isLastInstr(StableInstrConstIter it) const {
  return (it == getLastInstrIter());
}

BCBuilder::StableInstrIter BCBuilder::getLastInstrIter() {
  assert(vector.size() && "not available for empty buffers");
  return StableInstrIter(vector, vector.size()-1);
}

BCBuilder::StableInstrConstIter BCBuilder::getLastInstrIter() const {
  assert(vector.size() && "not available for empty buffers");
  return StableInstrConstIter(vector, vector.size()-1);
}

void BCBuilder::addDebugRange(StableInstrConstIter iter, SourceRange range) {
  assert(debugInfo && "no debug info!");

  auto ptr = iter.getContainerIterator();
  assert(ptr != vector.end() 
    && "iter is the end iterator");

  std::size_t idx = std::distance((const Instruction*)vector.begin(), ptr);
  
  debugInfo->addSourceRange(idx, range);
}

void BCBuilder::popInstr() {
  vector.pop_back();
}

BCBuilder::StableInstrIter BCBuilder::insert(Instruction instr) {
  vector.push_back(instr);
  return getLastInstrIter();
}