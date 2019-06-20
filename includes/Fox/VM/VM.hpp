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
#include "Fox/Common/BuiltinKinds.hpp"
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
  class Diagnostic;
  class DiagnosticEngine;
  enum class DiagID : std::uint16_t;
  class Object;
  class StringObject;
  class ArrayObject;

  class VM {
    public:
      ///--------------------------------------------------------------------///
      /// VM Nested Classes
      ///--------------------------------------------------------------------///

      /// A Function reference: a discriminated union of a BCFunction*
      /// and a BuiltinKind.
      class FunctionRef {
        public:
          FunctionRef(BCFunction* fn) { *this = fn; }
          FunctionRef(BuiltinKind id) { *this = id; }

          FunctionRef& operator=(BCFunction* fn) {
            data_ = fn;
            return *this;
          }
          FunctionRef& operator=(BuiltinKind id) {
            data_ = static_cast<BuiltinKind_t>(id);
            return *this;
          }

          /// \returns true if this FunctionRef contains a BCFunction*
          bool isBCFunction() const {
            return data_.is<BCFunction*>();
          }

          /// \returns true if this FunctionRef contains a BuiltinKind
          bool isBuiltin() const {
            return data_.is<EmbeddedID>();
          }

          /// \returns the BCFunction* contained in this function
          /// (isBCFunction() must return true)
          BCFunction* getBCFunction() {
            return data_.get<BCFunction*>();
          }

          /// \returns the BuiltinKind contained in this function
          /// (isBuiltin() must return true)
          BuiltinKind getBuiltinKind() {
            return BuiltinKind((BuiltinKind_t)data_.get<EmbeddedID>());
          }

        private:
          using EmbeddedID = llvm::PointerEmbeddedInt<BuiltinKind_t>;
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
        explicit Register(BuiltinKind v)       : funcRef(v) {}
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
      /// \p args the arguments array
      /// \return the return value of the executed function.
      Register run(BCFunction& func, ArrayRef<Register> args);
                 
      /// Calls a function \p func
      /// \return the return value of the executed function.
      Register run(BCFunction& func);

      /// Executes a bytecode buffer \p instrs.
      /// \return the return value of the executed instruction buffer.
      Register run(ArrayRef<Instruction> instrs);

      /// \returns the program counter
      const Instruction* getPC() const;

      /// Emits a diagnostic at the current instruction's location.
      /// \returns the diagnostic object (see DiagnosticEngine::report)
      Diagnostic diagnose(DiagID diag) const;

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

      /// \returns a view of the array containing the global variables
      ArrayRef<Register> getGlobalVariables() const;

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

      /// Creates a new ArrayObject intended to store value types,
      /// with \p reservedElems reserved elements.
      LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS
      ArrayObject* newValueArrayObject(std::size_t reservedElems = 0);

      /// Creates a new ArrayObject intended to store reference types,
      /// with \p reservedElems reserved elements.
      LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS
      ArrayObject* newRefArrayObject(std::size_t reservedElems = 0);

      ///--------------------------------------------------------------------///
      /// Public Member Variables
      ///--------------------------------------------------------------------///

      /// The Bytecode module executed by this VM instance.
      BCModule& bcModule;

      /// The DiagnosticEngine used to emit runtime diagnostics
      DiagnosticEngine& diagEngine;

    private:
      /// Creates the register array for the global variables and runs the
      /// initializers.
      void initGlobals();

      /// \returns the number of global variables available
      std::size_t numGlobals() const;

      /// Internal method to handle calls to a function
      /// \param base the base register of the call
      /// \returns a pointer to the register containing the return value
      /// of the executed bytecode. nullptr if there is no return value
      /// (void)
      Register callFunc(regaddr_t base);

      /// Internal method to run a builtin function in the current window.
      Register callBuiltinFunc(BuiltinKind id);

      /// \returns a reference to the register at address \p idx in the current
      /// register window.
      Register& getReg(regaddr_t idx) {
        assert((idx < numStackRegister) && "out-of-range");
        return baseReg_[idx];
      }

      /// \returns a reference to the global variable with id \p id
      Register& getGlobal(global_id_t id) {
        assert((id < numGlobals()) && "out-of-range");
        return globals_[id];
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
      /// Global variable registers
      std::unique_ptr<Register[]> globals_;

      // Temporarily, objects are simply allocated in vectors of unique_ptrs
      // for simplicity. I'll implement a more elaborate allocation technique
      // when the GC is implemented.
      // For now, objects are simply never freed.
      SmallVector<std::unique_ptr<StringObject>, 4> stringObjects_;
      SmallVector<std::unique_ptr<ArrayObject>, 4> arrayObjects_;
  };
}