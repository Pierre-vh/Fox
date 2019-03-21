//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Instruction.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file contains the bytecode instructions-related classes.
//----------------------------------------------------------------------------//

#pragma once

#include "BCUtils.hpp"
#include "llvm/Support/Compiler.h"
#include <cstdint>
#include <iosfwd>

namespace llvm {
  template<typename T> class ArrayRef;
}

namespace fox {
  struct Instruction;

  // VM Op codes.
  enum class Opcode : opcode_t {
    #define INSTR(Op) Op,
    #define LAST_INSTR(Op) last_opcode = Op
    #include "Instruction.def"
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

      #include "Instruction.def"
    };
  };
  LLVM_PACKED_END

  static_assert(sizeof(Instruction) == 4, "Size of 'Instruction' object not"
    " 4 Bytes/32 bits!");
}