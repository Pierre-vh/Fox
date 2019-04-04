//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : VM.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This contains the Fox Virtual Machine (VM), which runs Fox bytecode
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/BC/BCUtils.hpp"
#include "Fox/Common/FoxTypes.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/MathExtras.h"
#include <cstdint>
#include <cstddef>
#include <array>

namespace fox {
  struct Instruction;
  class BCModule;
  class VM {
    public:
      /// The number of registers on the register stack
      static constexpr unsigned numStackRegister = 255;

      /// \param bcModule the BCModule to execute
      VM(BCModule& bcModule);

      /// executes/runs the current module
      void run();

      /// \returns the program counter as an index in the module's
      /// Bytecode buffer.
      std::size_t getPCIndex() const;

      /// \returns the program counter as a pointer into the module's
      /// instruction buffeR.
      const Instruction* getPC() const;

      /// \returns a view of the register stack
      ArrayRef<std::uint64_t> getRegisterStack() const;

      /// \returns a mutable view of the register stack
      MutableArrayRef<std::uint64_t> getRegisterStack();

      /// The Bytecode module executed by this VM instance.
      BCModule& bcModule;

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

      // The program counter
      const Instruction* programCounter_ = nullptr;

      // The registers
      std::array<std::uint64_t, numStackRegister> regStack_ = {0};
  };
}