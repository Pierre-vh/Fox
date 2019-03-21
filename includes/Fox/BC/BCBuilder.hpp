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
      // The type of an instruction buffer.
      using Buffer = InstructionBuffer;

      BCModuleBuilder();
      ~BCModuleBuilder();

      #define TERNARY_INSTR(ID, T1, T2, T3)\
        BCModule::instr_iterator create##ID##Instr(T1 arg0, T2 arg1, T3 arg2);
      #define BINARY_INSTR(ID, T1, T2)\
        BCModule::instr_iterator create##ID##Instr(T1 arg0, T2 arg1);
      #define UNARY_INSTR(ID, T1)\
        BCModule::instr_iterator create##ID##Instr(T1 arg);
      #define SIMPLE_INSTR(ID)\
        BCModule::instr_iterator create##ID##Instr();
      #include "Instruction.def"

      // Takes the current BCModule from the builder.
      // NOTE: The builder will lazily create another module
      // when getModule (const or not) is called.
      std::unique_ptr<BCModule> takeModule();
      BCModule& getModule();
      const BCModule& getModule() const;

    private:
      std::unique_ptr<BCModule> bcModule_;
  };
}