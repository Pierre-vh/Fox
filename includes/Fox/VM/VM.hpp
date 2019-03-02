//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : VM.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This contains the Fox Virtual Machine (VM), which runs Fox bytecode
//----------------------------------------------------------------------------//

#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include "llvm/ADT/ArrayRef.h"
#include "Fox/Common/LLVM.hpp"

namespace fox {
  class VM {
    public:
      static constexpr unsigned numStackRegister = 255;

      // Loads a program
      void load(ArrayRef<std::uint32_t> instrs);

      // Runs the current program.
      void run();

      ArrayRef<std::uint64_t> getRegisterStack() const;
      MutableArrayRef<std::uint64_t> getRegisterStack();

    private:
      // The program currently being executed
      ArrayRef<std::uint32_t> program_;

      // The program counter
      std::uint64_t programCounter_ = 0;

      // The registers
      std::array<std::uint64_t, numStackRegister> regStack_;
  };
}