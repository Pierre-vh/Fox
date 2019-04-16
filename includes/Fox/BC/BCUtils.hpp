//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BCUtils.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file contains some general bytecode-related utilities:
//  forward declarations, constants and type-aliases for use by the bytecode
//  classes. It also contains the 'bc_limits' namespace that contains various
//  constants related to bytecode limitations.
//
//  This allows clients to get some information about the bytecode without
//  having to include larger files such as Instruction.hpp
//----------------------------------------------------------------------------//

#pragma once

#include <cstdint>
#include <cstddef>

namespace llvm {
  template <typename T, unsigned N> class SmallVector;
}

namespace fox {
  struct Instruction;

  /// The type of a register address in an instruction
  using regaddr_t = std::uint8_t;

  /// The type of the jump offset value for Jump, JumpIf and
  /// JumpIfNot.
  using jump_offset_t = std::int16_t;

  /// The type of a constant 'ID' (the unique identifier of a
  /// constant of a certain type)
  using constant_id_t = std::uint16_t;

  /// The type of a function 'ID' (the unique identifier 
  /// of a function)
  using func_id_t = std::uint16_t;

  /// The underlying type of the 'Opcode' enum
  using opcode_t = std::uint8_t;

  /// Forward declaration of the 'Opcode' enum
  enum class Opcode : opcode_t;

  /// A Vector of Instructions
  using InstructionVector = llvm::SmallVector<Instruction, 4>;

  namespace bc_limits {
    /// the maximum number of functions that can be contained
    /// in a single BCModule.
    constexpr std::size_t max_functions = 0xFFFF;

    /// the maximum constant ID possible
    constexpr constant_id_t max_constant_id = 0xFF;

    /// the maximum function ID possible
    constexpr func_id_t max_func_id = 0xFF;

    /// the maximum jump offset possible (positive or negative) for Jump, JumpIf
    /// and JumpIfNot.
    constexpr jump_offset_t max_jump_offset = (1 << 15)-1;

    /// the minimum jump offset possible for Jump, JumpIf and JumpIfNot.
    constexpr jump_offset_t min_jump_offset = -max_jump_offset;

    /// the maximum register address possible.
    constexpr regaddr_t max_regaddr = 0xFF;
  }
}