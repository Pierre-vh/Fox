//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Instruction.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BC/Instruction.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/Common/Errors.hpp"
#include "llvm/ADT/ArrayRef.h"

using namespace fox;

static constexpr const char* const opcodeStrings[] = {
  #define INSTR(Op) #Op,
  #include "Fox/BC/Instruction.def"
};

static bool isLegal(Opcode op) {
  return std::uint8_t(op) <= std::uint8_t(Opcode::last_opcode);
}

const char* fox::toString(Opcode op) {
  if(isLegal(op))
    return opcodeStrings[std::uint8_t(op)];
  return nullptr;
}

//----------------------------------------------------------------------------//
// instruction dumps
//----------------------------------------------------------------------------//

namespace {
  /// Because we use the unary+ to uprank uint8/int8 to a int (so it doesn't
  /// print rubbish), we need to specify it for BuiltinID so it works with
  /// it too.
  BuiltinID operator+(BuiltinID id) {
    return id;
  }
}

void fox::dumpInstruction(std::ostream& os, Instruction instr) {
  #define SIMPLE_INSTR(ID)\
    case Opcode::ID: os << #ID; break;
  #define TERNARY_INSTR(ID, I1, T1, I2, T2, I3, T3)\
    case Opcode::ID: os << #ID << " " << +instr.ID.I1 << " "\
                        << +instr.ID.I2 << " " << +instr.ID.I3; break;
  #define BINARY_INSTR(ID, I1, T1, I2, T2)\
    case Opcode::ID: os << #ID << " " << +instr.ID.I1 << " "\
                        << +instr.ID.I2; break;
  #define UNARY_INSTR(ID, I1, T1)\
    case Opcode::ID: os << #ID << " " << +instr.ID.I1; break;
  switch (instr.opcode) {
    #include "Fox/BC/Instruction.def"
    default:
      os << "<invalid opcode>";
  }
  #undef CASE
  os << '\n';
}

void fox::dumpInstructions(std::ostream& os, ArrayRef<Instruction> instrs, 
                           const char* linePrefix) {
  std::size_t count = 0;
  for (auto instr : instrs) {
    os << linePrefix << " " << (count++) << "\t| ";
    dumpInstruction(os, instr);
  }
}
