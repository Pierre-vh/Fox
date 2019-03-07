//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Opcode.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/VM/Instructions.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/Common/Errors.hpp"
#include "llvm/ADT/ArrayRef.h"

using namespace fox;

static constexpr const char* const opcodeStrings[] = {
  #define INSTR(Op) #Op,
  #include "Fox/VM/Instructions.def"
};

const char* fox::toString(Opcode op) {
  if(isLegalOpcode(op))
    return opcodeStrings[static_cast<std::uint8_t>(op)];
  return nullptr;
}

//----------------------------------------------------------------------------//
// instruction dumps
//----------------------------------------------------------------------------//

namespace {
  // Prints a TERNARY_INSTR
  void dumpTernaryInstr(std::ostream& os, const char* opcStr, std::uint32_t instr) {
    os << opcStr << " ";
    // Print 'a'
    os << +static_cast<std::uint8_t>((instr & 0x0000FF00) >> 8) << " ";
    // Print 'b'
    os << +static_cast<std::uint8_t>((instr & 0x00FF0000) >> 16) << " ";
    // Print 'c'
    os << +static_cast<std::uint8_t>((instr & 0xFF000000) >> 24);
  }

  enum class BinaryInstrKind {
    Classic, Signed, Small
  };

  // Prints a BINARY_INSTR
  void 
  dumpBinaryInstr(std::ostream& os, const char* opcStr, std::uint32_t instr, 
                  BinaryInstrKind kind) {
    os << opcStr << " ";
    // Print 'a'
    os << +static_cast<std::uint8_t>((instr & 0x0000FF00) >> 8) << " ";
    // Print 'd'
    switch (kind) {
      case BinaryInstrKind::Classic:
        os << static_cast<std::uint16_t>((instr & 0xFFFF0000) >> 16);
        return;
      case BinaryInstrKind::Signed:
        os << static_cast<std::int16_t>((instr & 0xFFFF0000) >> 16);
        return;
      case BinaryInstrKind::Small:
        os << +static_cast<std::uint8_t>((instr & 0x00FF0000) >> 16);
        return;
      default: 
        fox_unreachable("unknown BinaryInstrKind");
    }
  }

  // Prints a UNARY_INSTR
  void dumpUnaryInstr(std::ostream& os, const char* opcStr, std::uint32_t instr,
                      bool isSigned) {
    os << opcStr << " ";
    if(isSigned)
      os << llvm::SignExtend32<24>((instr & 0xFFFFFF00) >> 8);
    else 
      os << static_cast<std::uint32_t>((instr & 0xFFFFFF00) >> 8);
  }
}

void fox::dumpInstruction(std::ostream& os, std::uint32_t instr) {
  #define CASE(ID, ACTION) case Opcode::ID: ACTION; break;
  #define SIMPLE_INSTR(ID) CASE(ID, os << #ID)
  #define TERNARY_INSTR(ID) CASE(ID, dumpTernaryInstr(os, #ID, instr))
  #define BINARY_INSTR(ID)\
    CASE(ID, dumpBinaryInstr(os, #ID, instr, BinaryInstrKind::Classic))
  #define SIGNED_BINARY_INSTR(ID)\
    CASE(ID, dumpBinaryInstr(os, #ID, instr, BinaryInstrKind::Signed))
  #define SMALL_BINARY_INSTR(ID)\
    CASE(ID, dumpBinaryInstr(os, #ID, instr, BinaryInstrKind::Small))
  #define UNARY_INSTR(ID)\
    CASE(ID, dumpUnaryInstr(os, #ID, instr, /*signed*/ false))
  #define SIGNED_UNARY_INSTR(ID)\
    CASE(ID, dumpUnaryInstr(os, #ID, instr, /*signed*/ true))
  switch (static_cast<Opcode>(instr & 0x000000FF)) {
    #include "Fox/VM/Instructions.def"
    default:
      os << "<invalid opcode>";
  }
  #undef CASE
}

void fox::dumpInstructions(std::ostream& os, ArrayRef<std::uint32_t> instrs) {
  bool first = true;
  for (auto instr : instrs) {
    if(first) first = false;
    else os << "\n";
    dumpInstruction(os, instr);
  }
}
