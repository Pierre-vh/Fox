//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : VM.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This contains interface to the Fox Virtual Machine (VM), which executes
//  Fox Bytecode.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/BC/BCUtils.hpp"
#include "Fox/Common/BuiltinID.hpp"
#include "Fox/Common/FoxTypes.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/PointerEmbeddedInt.h"
#include "llvm/ADT/PointerUnion.h"
#include <cstdint>
#include <cstddef>
#include <array>

namespace fox {
  struct Instruction;
  class BCModule;
  class BCFunction;
  class VM {
    public:
      /// A Function reference: a discriminated union of a BCFunction*
      /// and a BuiltinID.
      class FunctionRef {
        public:
          FunctionRef(BCFunction* fn) { *this = fn; }
          FunctionRef(BuiltinID id)   { *this = id; }

          FunctionRef& operator=(BCFunction* fn) {
            data_ = fn;
            return *this;
          }
          FunctionRef& operator=(BuiltinID id) {
            data_ = static_cast<builtin_id_t>(id);
            return *this;
          }

          /// \returns true if this FunctionRef contains a BCFunction*
          bool isBCFunction() const {
            return data_.is<BCFunction*>();
          }

          /// \returns true if this FunctionRef contains a BuiltinID
          bool isBuiltinID() const {
            return data_.is<EmbeddedID>();
          }

          /// \returns the BCFunction* contained in this function
          /// (isBCFunction() must return true)
          BCFunction* getBCFunction() {
            return data_.get<BCFunction*>();
          }

          /// \returns the BuiltinID contained in this function
          /// (isBuiltinID() must return true)
          BuiltinID getBuiltinID() {
            return BuiltinID((builtin_id_t)data_.get<EmbeddedID>());
          }

        private:
          using EmbeddedID = llvm::PointerEmbeddedInt<builtin_id_t>;
          llvm::PointerUnion<BCFunction*, EmbeddedID> data_;
      };

      /// Untagged union representing a single register value.
      /// This is exactly 8 bytes (64 Bits) in size.
      union Register {
        Register()                  : raw(0) {}
        Register(std::uint64_t raw) : raw(raw) {}
        Register(FoxInt v)          : intVal(v) {}
        Register(FoxDouble v)       : doubleVal(v) {}
        Register(BCFunction* v)     : funcRef(v) {}
        Register(BuiltinID v)       : funcRef(v) {}

        std::uint64_t raw;
        FoxInt intVal;
        FoxDouble doubleVal;
        FunctionRef funcRef;
      };

      static_assert(sizeof(Register) == 8,
        "The size of a Register is not 64 Bits");

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
      Register* call(BCFunction& func, 
                  MutableArrayRef<Register> args = MutableArrayRef<Register>());

      /// Executes a bytecode buffer \p instrs.
      /// \returns a pointer to the register containing the return value
      /// of the executed bytecode. nullptr if there is no return value
      /// (void)
      Register* run(ArrayRef<Instruction> instrs);

      /// \returns the program counter
      const Instruction* getPC() const;

      /// \returns a view of the register stack
      /// Note: this might be invalidated if a reallocation occurs.
      /// Do not trust the pointer after code has been run, functions
      /// have been called, etc.
      ArrayRef<Register> getRegisterStack() const;

      /// \returns a mutable view of the register stack
      /// Note: this might be invalidated if a reallocation occurs.
      /// Do not trust the pointer after code has been run, functions
      /// have been called, etc.
      MutableArrayRef<Register> getRegisterStack();

      /// The Bytecode module executed by this VM instance.
      BCModule& bcModule;

    private:
      /// Internal method to handle calls to a function
      /// \param base the base register of the call
      /// \returns a pointer to the register containing the return value
      /// of the executed bytecode. nullptr if there is no return value
      /// (void)
      Register* callFunc(regaddr_t base);

      /// Internal method to run a builtin function in the current window.
      Register callBuiltinFunc(BuiltinID id);

      /// \returns a reference to the register at address \p idx in the current
      /// register window.
      Register& getReg(regaddr_t idx) {
        assert((idx < numStackRegister) && "out-of-range");
        return baseReg_[idx];
      }

      /// \returns the address of the register at \p idx in the current
      /// window.
      Register* getRegPtr(regaddr_t idx) {
        return baseReg_+idx;
      }

      /// The Program Counter
      const Instruction* pc_ = nullptr;

      /// The register stack
      std::array<Register, numStackRegister> regStack_;
      /// The base register (rO) of the current function's register window.
      Register* baseReg_ = nullptr;
  };
}