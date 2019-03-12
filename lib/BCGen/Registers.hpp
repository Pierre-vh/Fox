//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Registers.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the RegisterAllocator and the RegisterValue classes.
//----------------------------------------------------------------------------//

#include <cstdint>

namespace fox {
  class RegisterValue;

  // An integer representing a register number
  using regnum_t = std::uint8_t;

  // The (per function) register allocator. It manages allocation
  // a freeing of registers, striving to reuse registers (at smaller indexes)
  // and making register management as efficient as possible.
  class RegisterAllocator {
    public:
      // Allocates a new register, returning a RegisterValue managing the
      // register.
      RegisterValue allocateNewRegister();

    private:
      friend RegisterValue;

      // Allocates a new register.
      // This should be used carefully as it returns the raw register
      // number. If that number is lost and freeRegister is never called,
      // the register will never be freed (like a memory leak)
      regnum_t rawAllocateNewRegister();

      // Marks the register 'reg' as being free and available again.
      void markRegisterAsFreed(regnum_t reg);

      // TODO: compactFreedRegs() method that tries to decrease the 'highest'
      // allocated register and remove stuff from the set of freed
      // registers. Maybe run this every X frees/allocs, or at each free?
      // (depends on how cheap the method is)

      // TODO: Count of 'highest' register allocated.
      // TODO: Set of freed register
  };

  // The register value, representing (maybe shared) ownership
  // of a single register number. It has semantics similar to std::unique_ptr.
  //
  // When destroyed, this class frees the register it's managing (or, in the
  // future, decreases its use count)
  class RegisterValue {
    public:
      RegisterValue(RegisterValue&& other);
      ~RegisterValue();

      RegisterValue& operator=(RegisterValue&& other);

      // Returns the number of the register managed by this Registervalue.
      regnum_t getRegisterNumber() const;

      // Returns true if this RegisterValue is still alive and
      // working.
      bool isAlive() const;

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
      regnum_t regNum_ = 0;
  };
}