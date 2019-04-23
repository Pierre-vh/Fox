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
      /// The type of a register
      using reg_t = std::uint64_t;

      /// The number of registers on the register stack
      static constexpr unsigned numStackRegister = 255;

      /// \param bcModule the bytecode module. This will serve as the context
      ///        of execution. Constants, Functions and everything else that
      ///        might be needed during the execution of bytecode will be
      ///        fetched in that module.
      VM(BCModule& bcModule);

      /// Executes a bytecode buffer \p instrs.
      /// \returns a pointer to the register containing the return value
      /// of the executed bytecode. nullptr if there is no return value
      /// (void)
      reg_t* run(ArrayRef<Instruction> instrs);

      /// \returns the program counter as an index in the module's
      /// Bytecode buffer.
      std::size_t getPCIndex() const;

      /// \returns the program counter as a pointer into the module's
      /// instruction buffeR.
      const Instruction* getPC() const;

      /// \returns a view of the register stack
      ArrayRef<reg_t> getRegisterStack() const;

      /// \returns a mutable view of the register stack
      MutableArrayRef<reg_t> getRegisterStack();

      /// The Bytecode module executed by this VM instance.
      BCModule& bcModule;

    private:
      // Returns the raw value of the register idx.
      reg_t getReg(regaddr_t idx) {
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

      void setupIterators(ArrayRef<Instruction> instrs);

      // The program counter
      const Instruction* instrsBeg_ = nullptr;
      const Instruction* curInstr_ = nullptr;

      // The registers
      std::array<reg_t, numStackRegister> regStack_ = {0};
  };
}