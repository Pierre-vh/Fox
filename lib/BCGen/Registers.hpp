//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Registers.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the RegisterAllocator and the RegisterValue classes.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/BC/BCUtils.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/PointerIntPair.h"
#include <cstdint>
#include <set>
#include <unordered_map>
#include <functional>

namespace llvm {
  template<typename T> class Optional;
}

namespace fox {
  class RegisterValue;
  class VarDecl;
  class LoopContext;

  // The (per function) register allocator. It manages allocation
  // a freeing of registers, striving to reuse registers (at smaller indexes)
  // and making register management as efficient as possible.
  class alignas(8) RegisterAllocator {
    public:
      // Preparation/Prologue methods

      // Initializes or Increments the use count of "var" in this
      // RegisterAllocator.
      void addUsage(const VarDecl* var);

      // Usage/Register allocation methods

      // Returns a RegisterValue managing a use of "var". When the RV dies,
      // the usage count of "var" is decremented, potentially
      // freeing it if it reaches zero.
      //
      // This asserts that this is the first usage (the declaration) of
      // var.
      //
      // Optionally, an hint can be provided. (Hint should be a register
      // where .canRecycle() returns true). When possible,
      // this method will recycle the hint, storing the variable inside
      // hint instead of allocating a new register for it.
      RegisterValue initVar(const VarDecl* var, RegisterValue* hint = nullptr);

      // Returns a RegisterValue managing a use of "var". When the RV dies,
      // the usage count of "var" is decremented, potentially
      // freeing it if it reaches zero.
      //
      // This asserts that Var has already been declared (that it has an 
      // address) and that it is alive.
      RegisterValue useVar(const VarDecl* var);

      // Allocates a new temporary register, returning a RegisterValue 
      // managing the register. Once the RegisterValue dies, the register
      // is freed.
      RegisterValue allocateTemporary();

      // Recycle a register that's about to die, transforming it into
      // a temporar that has the same address.
      RegisterValue recycle(RegisterValue value);
      
      // Returns the number of registers currently in use
      regaddr_t numbersOfRegisterInUse() const;

    private:
      friend RegisterValue;
      friend LoopContext;

      // Returns true if we are inside a loop (if we have an active LoopContext)
      bool isInLoop() const;

      // Performs some actions related to the destruction of a LoopContext.
      //    - Perform some checks : The use count of all variables in declaredIn
      //      should have reached zero.
      //    - Frees every register occupied by the variables in declaredIn
      void actOnEndOfLoopContext(LoopContext& lc);

      // This method will take care of recycling 'value', returning
      // its address.
      // This should be used carefully as it returns the raw register
      // address. If address number is lost and freeRegister is never called,
      // the register will never be freed (like a memory leak)
      regaddr_t rawRecycleRegister(RegisterValue value);

      // Allocates a new register.
      // This should be used carefully as it returns the raw register
      // address. If that address is lost and freeRegister is never called,
      // the register will never be freed (like a memory leak)
      regaddr_t rawAllocateNewRegister();

      // Marks the register 'reg' as being free and available again.
      void markRegisterAsFreed(regaddr_t reg);

      // Returns the register in which "var" is stored.
      // NOTE: This assert that we have assigned a register to this
      //       variable. It will NOT assign a register to that
      //       var if it doesn't have one. 
      //       (It just reads data)
      regaddr_t getRegisterOfVar(const VarDecl* var) const;

      // Decrements the use count of "var", potentially freeing
      // it if its use count reaches 0
      void release(const VarDecl* var, bool isAlreadyDead = false);

      // Returns true if we can recycle this variable
      bool canRecycle(const VarDecl* var) const;

      // This method tries to remove elements from the freeRegisters_
      // set by decrementing biggestAllocatedReg_.
      //
      // It is called every register allocation, in the future, this
      // may also be called before setting up a call to minimize
      // register usage.
      //
      // Example:
      // pre compacting:
      //    freeRegisters_ = (4, 3, 1, 0) and biggestAllocatedReg_ = 5
      // after compacting:
      //    freeRegisters_ = (1, 0) and biggestAllocatedReg_ = 3
      void compactFreeRegisterSet();

      // The address of the 'highest' allocated register + 1
      //
      // e.g. if we have allocated 5 registers, this value will be set to 6.
      regaddr_t biggestAllocatedReg_ = 0;

      // The set of free registers, sorted from the highest to the lowest
      // one.
      std::set<regaddr_t, std::greater<regaddr_t> > freeRegisters_;

      // The data of a VarDecl known by this RegisterAllocator.
      // This contains 2 fields: the (optional ) address of the variable
      //  (none if unassigned), and the variable's use count.
      // When the use count reaches 0, the register occupied by the variable
      // is freed.
      struct VarData {
        bool hasAddress() const {
          return addr.hasValue();
        }

        llvm::Optional<regaddr_t> addr;
        std::size_t useCount;
      };

      // The set of variables known by this RegisterAllocator.
      std::unordered_map<const VarDecl*, VarData> knownVars_;
      // The current LoopContext
      LoopContext* curLoopContext_ = nullptr;
  };

  // The register value, representing (maybe shared) ownership
  // of a single register number. It has semantics similar to std::unique_ptr.
  //
  // When destroyed, this class frees the register it is managing (or, in the
  // future, decreases its use count)
  class RegisterValue {
    using kind_t = std::uint8_t;
    public:
      enum class Kind : kind_t {
        // For RegisterValues that manage temporary variables, freeing the
        // register when they're destroyed.
        Temporary,
        // For RegisterValues that manage a reference to a variable and
        // will decrement the usage counter of that Var (potentially freeing the
        // register if it reaches 0) when destroyed
        Var,

        last_kind = Var
      };

      RegisterValue() = default;
      RegisterValue(RegisterValue&& other);
      ~RegisterValue();

      Kind getKind() const;

      RegisterValue& operator=(RegisterValue&& other);

      // Returns the 'address' of the register managed by this Registervalue.
      regaddr_t getAddress() const;

      // Returns true if this RegisterValue is still alive and
      // working.
      bool isAlive() const;

      // Returns true if getKind() == Kind::Temporary
      bool isTemporary() const;
      // Returns true if getKind() == Kind::Var
      bool isVar() const;

      // Returns true if this RegisterValue can be recycled by
      // RegisterAllocator::recycle or RegisterAllocator::initVar
      bool canRecycle() const;

      // Calls isAlive()
      explicit operator bool() const;

      // Frees the register and kills this RegisterValue.
      void free();

      // Disable the copy constructor and copy
      // assignement operator.
      RegisterValue(const RegisterValue&) = delete;
      RegisterValue& operator=(const RegisterValue&) = delete;

    private:  
      friend RegisterAllocator;

      RegisterAllocator* getRegisterAllocator();
      const RegisterAllocator* getRegisterAllocator() const;

      // Constructor for 'Temporary' RegisterValues
      RegisterValue(RegisterAllocator* regAlloc, regaddr_t reg);

      // Constructor for 'Var' RegisterValues
      RegisterValue(RegisterAllocator* regAlloc, const VarDecl* var);

      // "Kills" this RegisterValue, making it useless/ineffective.
      // Used by the move constructor/assignement operator.
      //
      // This should be used carefully because this will not
      // free the register. Use "free()" for that!
      void kill();

      // The number of bits used to store the Kind in regAllocAndKind_
      static constexpr unsigned kindBits = 1;

      static_assert(static_cast<kind_t>(Kind::last_kind) <= kindBits,
        "Not enough kindBits to represent all RegisterValue kinds");

      llvm::PointerIntPair<RegisterAllocator*, kindBits, Kind> regAllocAndKind_;
      union {
        const VarDecl* varDecl = nullptr;
        regaddr_t tempRegAddress;
      } data_;
  };
}