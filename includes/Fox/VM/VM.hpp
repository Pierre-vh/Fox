//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : VM.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This contains the Fox Virtual Machine (VM), which runs Fox bytecode
//----------------------------------------------------------------------------//

#pragma once

#include "VMUtils.hpp"
#include "Fox/Common/FoxTypes.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/MathExtras.h"
#include <cstdint>
#include <cstddef>
#include <array>

namespace fox {
  struct Instruction;

  class VM {
    public:
      static constexpr unsigned numStackRegister = 255;

      // Loads a program. 
      // Currently, the last instruction of the program must be a
      // "Break" instruction. This is enforced by an assertion.
      void load(ArrayRef<Instruction> instrs);

      // Runs the current program.
      void run();

      // Returns the current value of the program counter.
      std::size_t getPC() const;

      ArrayRef<std::uint64_t> getRegisterStack() const;
      MutableArrayRef<std::uint64_t> getRegisterStack();

    private:
      // Returns the raw value of the register idx.
      std::uint64_t getReg(regaddr_t idx) {
        assert((idx < numStackRegister) && "out-of-range");
        return regStack_[idx];
      }

      // Returns the value of the register "idx" reinterpreted as
      // a value of type Ty. 
      // Only available for types whose size is less or equal to 8 Bytes
      template<typename Ty>
      Ty getReg(regaddr_t idx) {
        static_assert(sizeof(Ty) <= 8,
          "Can't cast a register to a type larger than 64 bits");
        assert((idx < numStackRegister) && "out-of-range");
        return static_cast<Ty>(regStack_[idx]);
      }

      // Special overload of the templated getReg for doubles, becauses
      // doubles can't be just reinterpret-cast'd. 
      template<>
      FoxDouble getReg<FoxDouble>(regaddr_t idx) {
        assert((idx < numStackRegister) && "out-of-range");
        return llvm::BitsToDouble(regStack_[idx]);
      }

      // Sets the value of the register "idx" to value.
      void setReg(regaddr_t idx, std::uint64_t value) {
        assert((idx < numStackRegister) && "out-of-range");
        regStack_[idx] = value;
      }

      // Sets the value of the register "idx" to 
      // static_cast<std::uint64_t>(value).
      // Only available for types whose size is less or equal to 8 Bytes
      template<typename Ty>
      void setReg(regaddr_t idx, Ty value) {
        static_assert(sizeof(Ty) <= 8,
          "Can't put a type larger than 64 bits in a register");
        assert((idx < numStackRegister) && "out-of-range");
        regStack_[idx] = static_cast<std::uint64_t>(value);
      }

      // Special overload of the templated setReg for doubles, becauses
      // doubles can't be just reinterpret-cast'd. 
      template<>
      void setReg<FoxDouble>(regaddr_t idx, FoxDouble value) {
        assert((idx < numStackRegister) && "out-of-range");
        regStack_[idx] = llvm::DoubleToBits(value);
      }

      // The program currently being executed
      ArrayRef<Instruction> program_;

      // The program counter
      std::uint64_t programCounter_ = 0;

      // The registers
      std::array<std::uint64_t, numStackRegister> regStack_;
  };
}