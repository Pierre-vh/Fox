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
#include "Fox/Common/LLVM.hpp"
#include <memory>
#include <cstdint>

namespace fox {
  class BCModule;
  class BCModuleBuilder {
    public:
      BCModuleBuilder();
      ~BCModuleBuilder();

      // The type of an instruction buffer.
      using Buffer = InstructionBuffer;

      #define SIMPLE_INSTR(ID) BCModuleBuilder& create##ID##Instr();
      #define TERNARY_INSTR(ID, T1, T2, T3) BCModuleBuilder&\
        create##ID##Instr(T1 arg0, T2 arg1, T3 arg2);
      #define BINARY_INSTR(ID, T1, T2) BCModuleBuilder&\
        create##ID##Instr(T1 arg0, T2 arg1);
      #define UNARY_INSTR(ID, T1) BCModuleBuilder&\
        create##ID##Instr(T1 arg);
      #include "Instruction.def"

      // Returns the last instruction inserted in the module.
      Instruction getLastInstr() const;

      // Takes the current BCModule from the builder.
      // NOTE: The builder will lazily create another module
      // when getModule (const or not) is called.
      std::unique_ptr<BCModule> takeModule();
      BCModule& getModule();
      const BCModule& getModule() const;

    private:
      void pushInstr(Instruction instr);

      std::unique_ptr<BCModule> vmModule_;
  };
}