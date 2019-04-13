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

  /// VM Op codes
  enum class Opcode : opcode_t {
    #define INSTR(Op) Op,
    #define LAST_INSTR(Op) last_opcode = Op
    #include "Instruction.def"
  };

  /// Converts an Opcode to a human-readable string representation.
  /// \return A C-String of the Opcode's name, or nullptr if the
  /// opcode isn't valid.
  const char* toString(Opcode op);
  
  /// Dumps a single instruction to os
  void dumpInstruction(std::ostream& os, Instruction instr);

  /// Dumps an array of instructions to os. Each instruction is dumped like this:
  /// \verbatim
  ///   index | Instr args...
  /// e.g.
  ///   0   | StoreSmallInt 0 0
  /// \endverbatim
  /// \param os the output stream
  /// \param instrs the instruction vector to dump
  /// \param linePrefix (optional) the prefix that should be printed before each
  ///        line.
  void dumpInstructions(std::ostream& os, llvm::ArrayRef<Instruction> instrs, 
                        const char* linePrefix = "");

  /// An object representing a single Fox instruction.
  ///
  /// The data of every non-simple instruction can be accessed
  /// using "object.opcode.data", e.g. instr.SmallStore.value.
  LLVM_PACKED_START
  struct alignas(std::uint32_t) Instruction {
    Instruction() = default;
    Instruction(Opcode op) : opcode(op) {}

    /// \returns true if this instruction is any kind of jump.
    bool isAnyJump() const {
      switch (opcode) {
        case Opcode::Jump:
        case Opcode::JumpIf:
        case Opcode::JumpIfNot:
          return true;
        default:
          return false;
      }
    }

    /// The Opcode
    Opcode opcode = Opcode::NoOp;

    union {
      #define TERNARY_INSTR(ID, I1, T1, I2, T2, I3, T3)       \
      struct ID##Instr {                                      \
        T1 I1;                                                \
        T2 I2;                                                \
        T3 I3;                                                \
      };                                                      \
      ID##Instr ID;                                           \
      static_assert(sizeof(ID##Instr) <= 3, #ID "Instr has "  \
        "too many arguments: their total size exceed 3 bytes/"\
        "24 bits!");

      #define BINARY_INSTR(ID, I1, T1, I2, T2)                \
      struct ID##Instr {                                      \
        T1 I1;                                                \
        T2 I2;                                                \
      };                                                      \
      ID##Instr ID;                                           \
      static_assert(sizeof(ID##Instr) <= 3, #ID "Instr has "  \
        "too many arguments: their total size exceed 3 bytes/"\
        "24 bits!");

      #define UNARY_INSTR(ID, I1, T1)                         \
      struct ID##Instr {                                      \
        T1 I1;                                                \
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