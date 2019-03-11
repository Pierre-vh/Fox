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
#include "llvm/Support/Compiler.h"

namespace llvm {
  template<typename T> class ArrayRef;
}

namespace fox {
  struct Instruction;

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

  // Converts an Opcode to a human-readable string representation.
  // If the opcode is illegal, nullptr is returned instead.
  const char* toString(Opcode op);
  
  // Dumps a single instruction to "os".
  void dumpInstruction(std::ostream& os, Instruction instr);

  // Dumps a series of instructions to "os"
  void dumpInstructions(std::ostream& os, llvm::ArrayRef<Instruction> instrs);

  // An object representing a single Fox instruction.
  //
  // The data of every non-simple instruction can be accessed
  // using "<object>.<opcode>.<data>", e.g. instr.SmallStore.arg0.
  //
  // Arguments are always named "arg0, arg1 and arg2" for ternary instructions,
  // "arg0 and arg1" for binary instructions and just "arg" for unary ones.
  //
  // It is aligned in the same way as a 32 bits unsigned integer.
  LLVM_PACKED_START
  struct alignas(std::uint32_t) Instruction {
    Instruction() = default;
    Instruction(Opcode op) : opcode(op) {}

    Opcode opcode = Opcode::NoOp;
    union {
      #define TERNARY_INSTR(ID, T1, T2, T3)                   \
      struct ID##Instr {                                      \
        T1 arg0;                                              \
        T2 arg1;                                              \
        T3 arg2;                                              \
      };                                                      \
      ID##Instr ID;                                           \
      static_assert(sizeof(ID##Instr) <= 3, #ID "Instr has "  \
        "too many arguments: their total size exceed 3 bytes/"\
        "24 bits!");

      #define BINARY_INSTR(ID, T1, T2)                        \
      struct ID##Instr {                                      \
        T1 arg0;                                              \
        T2 arg1;                                              \
      };                                                      \
      ID##Instr ID;                                           \
      static_assert(sizeof(ID##Instr) <= 3, #ID "Instr has "  \
        "too many arguments: their total size exceed 3 bytes/"\
        "24 bits!");

      #define UNARY_INSTR(ID, T1)                             \
      struct ID##Instr {                                      \
        T1 arg;                                               \
      };                                                      \
      ID##Instr ID;                                           \
      static_assert(sizeof(ID##Instr) <= 3, #ID "Instr has "  \
        "too many arguments: their total size exceed 3 bytes/"\
        "24 bits!");

      #include "Instructions.def"
    };
  };
  LLVM_PACKED_END

  static_assert(sizeof(Instruction) == 4, "Size of 'Instruction' object not"
    " 4 Bytes/32 bits!");
}