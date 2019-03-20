//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BCUtils.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file contains some general bytecode-related utilities:
//  forward declarations, constants and type-aliases for use by the bytecode
//  classes.
//
//  This allows clients to get some information about the bytecode without
//  having to include larger files such as Instructions.hpp
//----------------------------------------------------------------------------//

#pragma once

#include <cstdint>

namespace llvm {
  template <typename T, unsigned N> class SmallVector;
}

namespace fox {
  struct Instruction;

  // The type of a register address in an instruction
  using regaddr_t = std::uint8_t;

  // The maximum register address possible in an instruction
  constexpr regaddr_t max_regaddr = 0xFF;

  // The underlying type of the 'Opcode' enum
  using opcode_t = std::uint8_t;

  // Forward declaration of the 'Opcode' enum
  enum class Opcode : opcode_t;

  // An instruction buffer
  using InstructionBuffer = llvm::SmallVector<Instruction, 4>;
}