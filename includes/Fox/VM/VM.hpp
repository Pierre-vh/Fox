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
  class BCFunction;
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

      /// Executes a function \p func with parameters \p args.
      /// This is intended as an entry point for clients, and not as an internal
      /// method to handle function calls (mainly because it copies arguments
      /// and doesn't slide the register window)
      /// \p args must be null, or its size must match func.numParameters()
      /// \returns a pointer to the register containing the return value
      /// of the executed bytecode. nullptr if there is no return value
      /// (void)
      reg_t* call(BCFunction& func, 
                  MutableArrayRef<reg_t> args = MutableArrayRef<reg_t>());

      /// Executes a bytecode buffer \p instrs.
      /// \returns a pointer to the register containing the return value
      /// of the executed bytecode. nullptr if there is no return value
      /// (void)
      reg_t* run(ArrayRef<Instruction> instrs);

      /// \returns the program counter
      const Instruction* getPC() const;

      /// \returns a view of the register stack
      /// Note: this might be invalidated if a reallocation occurs.
      /// Do not trust the pointer after code has been run, functions
      /// have been called, etc.
      ArrayRef<reg_t> getRegisterStack() const;

      /// \returns a mutable view of the register stack
      /// Note: this might be invalidated if a reallocation occurs.
      /// Do not trust the pointer after code has been run, functions
      /// have been called, etc.
      MutableArrayRef<reg_t> getRegisterStack();

      /// The Bytecode module executed by this VM instance.
      BCModule& bcModule;

    private:
      /// Internal method to handle calls to a function
      /// \param base the base register of the call
      /// \returns a pointer to the register containing the return value
      /// of the executed bytecode. nullptr if there is no return value
      /// (void)
      reg_t* callFunc(regaddr_t base);

      /// \returns the raw register at address \p idx in the current
      /// window.
      reg_t getReg(regaddr_t idx) {
        assert((idx < numStackRegister) && "out-of-range");
        return baseReg_[idx];
      }

      /// \returns the address of the register at \p idx in the current
      /// window.
      reg_t* getRegPtr(regaddr_t idx) {
        return baseReg_+idx;
      }

      /// \returns the value of the register \p idx reinterpreted as
      /// a value of type Ty. 
      /// Only available for types whose size is less or equal to 8 Bytes
      template<typename Ty>
      Ty getReg(regaddr_t idx) {
        static_assert(sizeof(Ty) <= 8,
          "Can't cast a register to a type larger than 64 bits");
        return static_cast<Ty>(baseReg_[idx]);
      }

      // Special overload of the templated getReg for doubles, because
      // doubles can't be just reinterpret-cast'd. 
      template<>
      FoxDouble getReg<FoxDouble>(regaddr_t idx) {
        return llvm::BitsToDouble(baseReg_[idx]);
      }

      // Sets the value of the register "idx" to value.
      void setReg(regaddr_t idx, std::uint64_t value) {
        baseReg_[idx] = value;
      }

      // Sets the value of the register "idx" to 
      // static_cast<std::uint64_t>(value).
      // Only available for types whose size is less or equal to 8 Bytes
      template<typename Ty>
      void setReg(regaddr_t idx, Ty value) {
        static_assert(sizeof(Ty) <= 8,
          "Can't put a type larger than 64 bits in a register");
        baseReg_[idx] = static_cast<std::uint64_t>(value);
      }

      // Special overload of the templated setReg for doubles, because
      // doubles can't be just reinterpret-cast'd. 
      template<>
      void setReg<FoxDouble>(regaddr_t idx, FoxDouble value) {
        baseReg_[idx] = llvm::DoubleToBits(value);
      }

      // Special overload of setReg for pointer types
      template<typename Ty>
      void setReg(regaddr_t idx, Ty* ptr) {
        baseReg_[idx] = reinterpret_cast<std::intptr_t>(ptr);
      }

      /// The Program Counter
      const Instruction* pc_ = nullptr;

      /// The register stack
      std::array<reg_t, numStackRegister> regStack_ = {0};
      /// The base register (rO) of the current function's register window.
      reg_t* baseReg_ = nullptr;
  };
}