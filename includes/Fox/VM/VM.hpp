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
#include "Fox/BC/BCFunction.hpp"
#include "Fox/Common/BuiltinID.hpp"
#include "Fox/Common/FoxTypes.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/Common/string_view.hpp"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/PointerEmbeddedInt.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Compiler.h"
#include <memory>
#include <cstdint>
#include <cstddef>
#include <array>

namespace fox {
  struct Instruction;
  class BCModule;
  class Object;
  class StringObject;
  class ArrayObject;

  class VM {
    public:
      ///--------------------------------------------------------------------///
      /// VM Nested Classes
      ///--------------------------------------------------------------------///

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
        Register()                           : raw(0) {}
        explicit Register(std::uint64_t raw) : raw(raw) {}
        explicit Register(FoxInt v)          : intVal(v) {}
        explicit Register(FoxDouble v)       : doubleVal(v) {}
        explicit Register(BCFunction* v)     : funcRef(v) {}
        explicit Register(BuiltinID v)       : funcRef(v) {}
        explicit Register(Object* v)         : object(v) {}

        friend bool operator==(Register lhs, Register rhs) {
          return lhs.raw == rhs.raw;
        }

        friend bool operator!=(Register lhs, Register rhs) {
          return lhs.raw != rhs.raw;
        }

        explicit operator bool() const {
          return raw;
        }

        std::uint64_t raw;
        FoxInt intVal;
        FoxDouble doubleVal;
        FunctionRef funcRef;
        Object* object;
      };

      static_assert(sizeof(Register) == 8,
        "The size of a Register is not 64 Bits");

      ///--------------------------------------------------------------------///
      /// VM Interface
      ///--------------------------------------------------------------------///

      /// \param bcModule the bytecode module. This will serve as the context
      ///        of execution. Constants, Functions and everything else that
      ///        might be needed during the execution of bytecode will be
      ///        fetched in that module.
      VM(BCModule& bcModule);
      ~VM();

      /// Make this class non copyable
      VM(const VM&) = delete;
      VM& operator=(const VM&) = delete;

      /// Executes a function \p func with parameters \p args.
      /// This is intended as an entry point for clients, and not as an internal
      /// method to handle function calls (mainly because it copies arguments
      /// and doesn't slide the register window)
      /// \p args must be null, or its size must match func.numParameters()
      /// \returns a pointer to the register containing the return value
      /// of the executed bytecode. nullptr if there is no return value
      /// (void)
      /// \return the return value of the executed function.
      Register call(BCFunction& func, 
                  MutableArrayRef<Register> args = MutableArrayRef<Register>());

      /// Executes a bytecode buffer \p instrs.
      /// \return the return value of the executed instruction buffer.
      Register run(ArrayRef<Instruction> instrs);

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

      ///--------------------------------------------------------------------///
      /// Object Allocation
      ///--------------------------------------------------------------------///

      /// Creates a new, blank StringObject, maybe from a string.
      LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS
      StringObject* newStringObject(string_view str = string_view());

      /// Loads a string constant with id \p kID and creates a 
      /// new StringObject with its content.
      LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS
      StringObject* newStringObjectFromK(constant_id_t kID);

      /// Creates a new ArrayObject intended to store values,
      /// with \p reservedElems reserved elements.
      LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS
      ArrayObject* newValueArrayObject(std::size_t reservedElems = 0);

      /// Creates a new ArrayObject intended to store references,
      /// with \p reservedElems reserved elements.
      LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS
      ArrayObject* newRefArrayObject(std::size_t reservedElems = 0);

      ///--------------------------------------------------------------------///
      /// Public Member Variables
      ///--------------------------------------------------------------------///

      /// The Bytecode module executed by this VM instance.
      BCModule& bcModule;

    private:
      /// Internal method to handle calls to a function
      /// \param base the base register of the call
      /// \returns a pointer to the register containing the return value
      /// of the executed bytecode. nullptr if there is no return value
      /// (void)
      Register callFunc(regaddr_t base);

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

      /// The number of registers on the register stack
      static constexpr unsigned numStackRegister = 255;
      /// The register stack
      std::array<Register, numStackRegister> regStack_;
      /// The base register (rO) of the current function's register window.
      Register* baseReg_ = nullptr;

      // Temporarily, objects are simply allocated in vectors of unique_ptrs
      // for simplicity. I'll implement a more elaborate allocation technique
      // when the GC is implemented.
      // For now, objects are simply never freed.
      SmallVector<std::unique_ptr<StringObject>, 4> stringObjects_;
      SmallVector<std::unique_ptr<ArrayObject>, 4> arrayObjects_;
  };
}