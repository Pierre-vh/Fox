//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BCBuilder.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file contains the class used to generate Bytecode Modules (BCModules)
//  usable by the Fox VM.
//----------------------------------------------------------------------------//

#pragma once

#include "BCUtils.hpp"
#include "Fox/BC/BCModule.hpp"
#include "Fox/Common/LLVM.hpp"
#include <memory>
#include <cstdint>

namespace fox {
  class BCModuleBuilder {
    public:
      // The type of an interator of instructions
      using instr_iterator = BCModule::instr_iterator;

      // The type of an instruction buffer.
      using Buffer = InstructionBuffer;

      BCModuleBuilder();
      ~BCModuleBuilder();

      #define TERNARY_INSTR(ID, I1, T1, I2, T2, I3, T3)\
        BCModule::instr_iterator create##ID##Instr(T1 I1, T2 I2, T3 I3);
      #define BINARY_INSTR(ID, I1, T1, I2, T2)\
        BCModule::instr_iterator create##ID##Instr(T1 I1, T2 I2);
      #define UNARY_INSTR(ID, I1, T1)\
        BCModule::instr_iterator create##ID##Instr(T1 I1);
      #define SIMPLE_INSTR(ID)\
        BCModule::instr_iterator create##ID##Instr();
      #include "Instruction.def"

      // Returns an iterator to the last instruction inserted
      // in this vector.
      instr_iterator getLastInstr();

      // Erases all instructions in the range [beg, end)
      void truncate_instrs(instr_iterator beg);

      // Returns true if 'it' == instrs_back().
      bool isLastInstr(instr_iterator it) const;

      // Removes the last instruction added to this module.
      void popInstr();

      // Takes the current BCModule from the builder.
      // NOTE: The builder will lazily create another module
      // when getModule (const or not) is called.
      std::unique_ptr<BCModule> takeModule();
      BCModule& getModule();
      const BCModule& getModule() const;

    private:
      InstructionBuffer& getInstrBuffer();
      const InstructionBuffer& getInstrBuffer() const;

      std::unique_ptr<BCModule> bcModule_;
  };
}