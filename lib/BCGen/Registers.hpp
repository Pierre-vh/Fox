//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Registers.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the RegisterAllocator and the RegisterValue classes.
//----------------------------------------------------------------------------//

#include <cstdint>
#include <set>
#include <functional>

namespace fox {
  class RegisterValue;

  // An integer representing a register number
  using regnum_t = std::uint8_t;

  // The (per function) register allocator. It manages allocation
  // a freeing of registers, striving to reuse registers (at smaller indexes)
  // and making register management as efficient as possible.
  class RegisterAllocator {
    public:
      // Allocates a new temporary register, returning a RegisterValue 
      // managing the register. Once the RegisterValue dies, the register
      // is freed.
      RegisterValue allocateTemporary();
      
      // Returns the number of registers currently in use
      regnum_t numbersOfRegisterInUse() const;

    private:
      friend RegisterValue;

      // The maximum number of registers that we can allocated=
      static constexpr 
      regnum_t maxRegNum = std::numeric_limits<regnum_t>::max();

      // Allocates a new register.
      // This should be used carefully as it returns the raw register
      // number. If that number is lost and freeRegister is never called,
      // the register will never be freed (like a memory leak)
      regnum_t rawAllocateNewRegister();

      // Marks the register 'reg' as being free and available again.
      void markRegisterAsFreed(regnum_t reg);

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

      // The biggest register number allocated.
      regnum_t biggestAllocatedReg_ = 0;

      // The set of free registers, sorted from the highest to the lowest
      // one.
      std::set<regnum_t, std::greater<regnum_t> > freeRegisters_;
  };

  // The register value, representing (maybe shared) ownership
  // of a single register number. It has semantics similar to std::unique_ptr.
  //
  // When destroyed, this class frees the register it's managing (or, in the
  // future, decreases its use count)
  class RegisterValue {
    public:
      RegisterValue() = default;
      RegisterValue(RegisterValue&& other);
      ~RegisterValue();

      RegisterValue& operator=(RegisterValue&& other);

      // Returns the 'address' of the register managed by this Registervalue.
      // The address is simply an integer for the register number.
      regnum_t getAddress() const;

      // Returns true if this RegisterValue is still alive and
      // working.
      bool isAlive() const;

      // Returns true if this RegisterValue is a temporary
      // one. If that's the case, the register will
      // be freed once this instance dies.
      bool isTemporary() const;

      // Frees the register and kills this RegisterValue.
      void free();

      // Disable the copy constructor and copy
      // assignement operator.
      RegisterValue(const RegisterValue&) = delete;
      RegisterValue& operator=(const RegisterValue&) = delete;

    private:  
      friend RegisterAllocator;

      // Constructor, called by RegisterAllocator.
      RegisterValue(RegisterAllocator* regAlloc, regnum_t reg);

      // "Kills" this RegisterValue, making it useless/ineffective.
      // Used by the move constructor/assignement operator.
      //
      // This should be used carefully because this will not
      // free the register. Use "free()" for that!
      void kill();

      RegisterAllocator* regAlloc_ = nullptr;
      regnum_t regAddress_ = 0;
  };
}