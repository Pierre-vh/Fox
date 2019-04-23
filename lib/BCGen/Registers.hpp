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

  /// The RegisterAllocator. Usually one of these is created for each
  /// function. The role of this class is to track free and used
  /// register and assign registers to variables and temporaries.
  /// It strives to use as little registers as possible.
  class alignas(8) RegisterAllocator {
    public:
      /// Preparation/Prologue methods

      /// Initializes a variable \p var in this RegisterAllocator.
      /// If \p var has already been initialized, increments its use count.
      void addUsage(const VarDecl* var);

      /// Usage/Register allocation methods

      /// Initializes a known variable \p var. \p var must be known by this 
      /// RegisterAllocator (= initialized using \ref addUsage in the
      /// prologue phase)
      ///
      /// \returns a RegisterValue managing a use of "var". When the RV dies,
      /// the usage count of "var" is decremented, potentially
      /// freeing it if it reaches zero.
      ///
      /// Optionally, a hint \p hint can be passed. It should be a recyclable
      /// register. If \p hint is non null, this method will reuse its register
      /// instead of allocating a new one for \p var.
      RegisterValue initVar(const VarDecl* var, RegisterValue* hint = nullptr);
      
      /// Use an already declared variable \p var. \p var must have been 
      /// initialized through \p useVar already.
      ///
      /// \returns a RegisterValue managing a use of "var". When the RV dies,
      /// the usage count of "var" is decremented, potentially
      /// freeing it if it reaches zero.
      RegisterValue useVar(const VarDecl* var);

      /// Allocates a new temporary register
      ///
      /// \returns a RegisterValue  managing the register. 
      /// Once the RegisterValue dies, the register is freed.
      RegisterValue allocateTemporary();

      /// Recycle a register that's about to die, transforming it into
      /// a temporary that has the same address.
      RegisterValue recycle(RegisterValue value);
      
      /// \returns the number of registers currently in use
      regaddr_t numbersOfRegisterInUse() const;

    private:
      friend RegisterValue;
      friend LoopContext;

      /// The data of a VarDecl known by this RegisterAllocator.
      /// This contains 2 fields: the (optional ) address of the variable
      /// (llvm::None if unassigned), and the variable's use count.
      /// When the use count reaches 0, the register occupied by the variable
      /// is freed.
      struct VarData {
        bool hasAddress() const {
          return addr.hasValue();
        }

        llvm::Optional<regaddr_t> addr;
        std::size_t useCount;
      };
      
      /// The type of the hashmap used to track known variables and map them
      /// to their \ref VarData
      using KnownVarsMap = std::unordered_map<const VarDecl*, VarData>;

      /// \returns true if we are inside a loop (if we have an active LoopContext)
      bool isInLoop() const;

      /// Performs some actions related to the destruction of a LoopContext.
      ///    - Runs some checks : The use count of all variables declared in
      ///      the loop should have reached zero.
      ///    - Frees the registers in the delayedFrees register.
      void actOnEndOfLoopContext(LoopContext& lc);

      /// Recycles a register address \p value and returns it.
      /// This must be used carefully because if the return value is lost, the
      /// register will 'leak' just like a memory leak.
      regaddr_t rawRecycleRegister(RegisterValue value);

      /// Forgets a known variable, removing it from the set of known variables and
      /// (optionally) removing it from the current LoopContext if it belongs
      /// in it.
      /// NOTE: This deletes the VarData of the variable
      /// NOTE: This DOES NOT free the register occupied by the variable.
      void forgetVariable(KnownVarsMap::iterator iter);

      /// Allocates a new register.
      /// \returns the address of the allocated register.
      /// This should be used carefully as it returns the raw register
      /// address. If that address is lost and freeRegister is never called,
      /// the register will never be freed (like a memory leak)
      regaddr_t rawAllocateNewRegister();

      /// Marks the register 'reg' as being free and available again.
      /// This will either decrement the \ref biggestAllocatedReg_ variable
      /// or add the register to the \ref freeRegisters_ set.
      void markRegisterAsFreed(regaddr_t reg);

      /// \returns the register address in which "var" is stored.
      /// NOTE: This assert that we have assigned a register to this
      ///       variable. It will NOT assign a register to that
      ///       var if it doesn't have one.  (It just reads data)
      regaddr_t getRegisterOfVar(const VarDecl* var) const;

      /// Decrements the use count of \p var, potentially freeing
      /// it if its use count reaches 0.
      /// If \p isAlreadyDead is true the variable's use count must
      /// be zero.
      void release(const VarDecl* var, bool isAlreadyDead = false);

      /// \returns true if we can recycle the register occupied by this
      /// variable
      bool canRecycle(const VarDecl* var) const;

      /// This method tries to remove elements from the freeRegisters_
      /// set by decrementing biggestAllocatedReg_.
      ///
      /// It is called every register allocation, in the future, this
      /// may also be called before setting up a call to minimize
      /// register usage.
      void compactFreeRegisterSet();

      /// The address of the 'highest' allocated register + 1
      ///
      /// e.g. if we have allocated 5 registers, this value will be set to 6.
      regaddr_t biggestAllocatedReg_ = 0;

      /// The set of free registers, sorted from the highest to the lowest one.
      std::set<regaddr_t, std::greater<regaddr_t> > freeRegisters_;

      /// The set of variables known by this RegisterAllocator.
      KnownVarsMap knownVars_;
      /// The current LoopContext
      LoopContext* curLoopContext_ = nullptr;
  };

  /// The register value, representing (maybe shared) ownership
  /// of a single register address. It has semantics similar to a 
  /// std::unique_ptr.
  ///
  /// When destroyed, this class frees the register it is managing
  /// (if it's a temporary) or releases it (decrements its use count) 
  /// (if it's a variable)
  class RegisterValue {
    using kind_t = std::uint8_t;
    public:
      enum class Kind : kind_t {
        /// For RegisterValues that manage temporary variables, freeing the
        /// register when they're destroyed.
        Temporary,
        /// For RegisterValues that manage a reference to a variable and
        /// will decrement the usage counter of that Var (potentially freeing the
        /// register if it reaches 0) when destroyed
        Var,

        last_kind = Var
      };

      RegisterValue() = default;
      RegisterValue(RegisterValue&& other);
      ~RegisterValue();

      Kind getKind() const;

      RegisterValue& operator=(RegisterValue&& other);

      /// \returns the 'address' of the register managed by this Registervalue.
      regaddr_t getAddress() const;

      /// \returns true if this RegisterValue is still 'alive'
      bool isAlive() const;

      /// \returns true if getKind() == Kind::Temporary
      bool isTemporary() const;
      /// \returns true if getKind() == Kind::Var
      bool isVar() const;

      /// \returns true if this RegisterValue can be recycled by
      /// RegisterAllocator::recycle or RegisterAllocator::initVar
      bool canRecycle() const;

      /// calls \ref isAlive
      explicit operator bool() const;

      /// Frees the register and kills this RegisterValue.
      void free();

      /// Disable the copy constructor and copy
      /// assignement operator.
      RegisterValue(const RegisterValue&) = delete;
      RegisterValue& operator=(const RegisterValue&) = delete;

    private:  
      friend RegisterAllocator;

      RegisterAllocator* getRegisterAllocator();
      const RegisterAllocator* getRegisterAllocator() const;

      /// Constructor for 'Temporary' RegisterValues
      RegisterValue(RegisterAllocator* regAlloc, regaddr_t reg);

      /// Constructor for 'Var' RegisterValues
      RegisterValue(RegisterAllocator* regAlloc, const VarDecl* var);

      /// "Kills" this RegisterValue, making it useless/ineffective.
      /// Used by the move constructor/assignement operator.
      ///
      /// This should be used carefully because this will not
      /// free the register. Use "free()" for that!
      void kill();

      /// The number of bits used to store the Kind in regAllocAndKind_
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