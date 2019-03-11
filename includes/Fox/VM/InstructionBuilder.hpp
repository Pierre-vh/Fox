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
  struct Instruction;

  class InstructionBuilder {
    public:
      // The type of an instruction buffer.
      using Buffer = SmallVector<Instruction, 4>;

      #define SIMPLE_INSTR(ID) InstructionBuilder& create##ID##Instr();
      #define TERNARY_INSTR(ID, T1, T2, T3) InstructionBuilder&\
        create##ID##Instr(T1 arg0, T2 arg1, T3 arg2);
      #define BINARY_INSTR(ID, T1, T2) InstructionBuilder&\
        create##ID##Instr(T1 arg0, T2 arg1);
      #define UNARY_INSTR(ID, T1) InstructionBuilder&\
        create##ID##Instr(T1 arg);
      #include "Instructions.def"

      void reset();
      Instruction getLastInstr() const;
      ArrayRef<Instruction> getInstrs() const;
      
      // Take the current instruction buffer. 
      //
      // Note: InstructionBuilder will lazily create a new buffer when needed.
      std::unique_ptr<Buffer> takeBuffer();

    private:
      void pushInstr(Instruction instr);

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