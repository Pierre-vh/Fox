//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Opcode.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file contains informations about the Fox VM instruction set.
//----------------------------------------------------------------------------//

#pragma once

#include <cstdint>
#include <iosfwd>

namespace llvm {
  template<typename T> class ArrayRef;
}

namespace fox {
  // This enum contains every opcode. A enum class is used instead of
  // a traditional C enum to avoid polluting the global namespace. 
  // 
  // This shouldn't be an issue since we can:
  //  - use static_cast<std::uint8_t>(op) to get the value of the opcode
  //  - use static_cast<Opcode>(num) to get an Opcode back from an int.
  // Both have no runtime cost.
  enum class Opcode : std::uint8_t {
    #define INSTR(Op) Op,
    #define LAST_INSTR(Op) last_opcode = Op
    #include "Instructions.def"
  };

  // Checks if an Opcode legal. This is useful to check the actual validity
  // of the Opcode if you converted an int to an Opcode.
  inline constexpr bool isLegalOpcode(Opcode op) {
    return op <= Opcode::last_opcode;
  }

  // Checks if an integer value is a legal opcode.
  inline constexpr bool isLegalOpcode(std::uint8_t op) {
    return op <= static_cast<std::uint8_t>(Opcode::last_opcode);
  }

  // Converts an Opcode to a human-readable string representation.
  // If the opcode is illegal, "<illegal opcode>" is returned instead.
  const char* toString(Opcode op);
  
  // Dumps a single instruction to "os".
  void dumpInstruction(std::ostream& os, std::uint32_t instr);

  // Dumps a series of instructions to "os"
  void dumpInstructions(std::ostream& os, llvm::ArrayRef<std::uint32_t> instrs);
}