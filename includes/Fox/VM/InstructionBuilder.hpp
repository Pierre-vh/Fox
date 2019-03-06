//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : InstructionBuilder.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file contains the InstructionBuilder, which is a class that helps
//  build instructions buffers readable by the VM.
//----------------------------------------------------------------------------//

#pragma once

#include <cstdint>
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/ArrayRef.h"
#include "Fox/Common/LLVM.hpp"
#include <memory>

namespace fox {
  enum class Opcode : std::uint8_t;

  class InstructionBuilder {
    public:
      // The type of an instruction buffer.
      using Buffer = SmallVector<std::uint32_t, 4>;

      #define SIMPLE_INSTR(ID) InstructionBuilder& create##ID##Instr();
      #define TERNARY_INSTR(ID) InstructionBuilder&\
        create##ID##Instr(std::uint8_t a, std::uint8_t b, std::uint8_t c);
      #define SMALL_BINARY_INSTR(ID) InstructionBuilder&\
        create##ID##Instr(std::uint8_t a, std::uint8_t b);
      #define BINARY_INSTR(ID) InstructionBuilder&\
        create##ID##Instr(std::uint8_t a, std::uint16_t d);
      #define SIGNED_BINARY_INSTR(ID) InstructionBuilder&\
        create##ID##Instr(std::uint8_t a, std::int16_t d);
      #define UNARY_INSTR(ID) InstructionBuilder&\
        create##ID##Instr(std::uint32_t val);
      #define SIGNED_UNARY_INSTR(ID) InstructionBuilder&\
        create##ID##Instr(std::int32_t val);
      #include "Instructions.def"

      void reset();
      std::uint32_t getLastInstr() const;
      ArrayRef<std::uint32_t> getInstrs() const;
      
      // Take the current instruction buffer. 
      //
      // Note: InstructionBuilder will lazily create a new buffer when needed.
      std::unique_ptr<Buffer> takeBuffer();

    private:
      InstructionBuilder& 
      createSimpleInstr(Opcode op);

      InstructionBuilder& 
      createABCInstr(Opcode op, std::uint8_t a, std::uint8_t b, std::uint8_t c);

      InstructionBuilder& 
      createADInstr(Opcode op, std::uint8_t a, std::uint16_t d);

      InstructionBuilder& 
      createSignedUnaryInstr(Opcode op, std::int32_t val);

      InstructionBuilder& 
      createUnaryInstr(Opcode op, std::uint32_t val);

      void pushInstr(std::uint32_t instr);

      // Return true if we have a buffer, or false if one will
      // be created on the next getBuffer() call.
      bool hasBuffer() const;

      // Returns the Buffer, creating a new one if needed.
      Buffer& getBuffer();

      // Returns the Buffer, creating a new one if needed.
      const Buffer& getBuffer() const;

      std::unique_ptr<Buffer> instrBuffer_;
  };
}