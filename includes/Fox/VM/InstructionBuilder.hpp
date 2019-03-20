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

#include "VMUtils.hpp"
#include "Fox/Common/LLVM.hpp"
#include <memory>
#include <cstdint>

namespace fox {
  class VMModule;
  class InstructionBuilder {
    public:
      InstructionBuilder();
      ~InstructionBuilder();

      // The type of an instruction buffer.
      using Buffer = InstructionBuffer;

      #define SIMPLE_INSTR(ID) InstructionBuilder& create##ID##Instr();
      #define TERNARY_INSTR(ID, T1, T2, T3) InstructionBuilder&\
        create##ID##Instr(T1 arg0, T2 arg1, T3 arg2);
      #define BINARY_INSTR(ID, T1, T2) InstructionBuilder&\
        create##ID##Instr(T1 arg0, T2 arg1);
      #define UNARY_INSTR(ID, T1) InstructionBuilder&\
        create##ID##Instr(T1 arg);
      #include "Instructions.def"

      // Returns the last instruction pushed to the module.
      Instruction getLastInstr() const;

      // Takes the current VMModule from the builder.
      // NOTE: The builder will create a new one when needed.
      std::unique_ptr<VMModule> takeModule();
      VMModule& getModule();
      const VMModule& getModule() const;

    private:
      void pushInstr(Instruction instr);

      std::unique_ptr<VMModule> vmModule_;
  };
}