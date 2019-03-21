//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BCModule.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file contains the BCModule class, which represents a VM program
//  that can be executed by the Fox VM. This can be considered the
//  top-level container for the bytecode.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/BC/BCUtils.hpp"
#include "Fox/BC/Instruction.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"
#include <iterator>
#include <iosfwd>

namespace fox {
  class BCModule {
    public:
      class instr_iterator;

      // Returns the number of instructions in the instruction buffer
      std::size_t numInstructions() const;

      // Returns a reference to the instruction buffer
      InstructionBuffer& getInstructionBuffer();

      // Returns a constant reference to the instruction buffer
      const InstructionBuffer& getInstructionBuffer() const;

      instr_iterator instrs_beg();

      // Dumps the module to 'out'
      void dumpModule(std::ostream& out) const;

    private:
      // TODO: Move this out in the .cpp (using PIMPL) to reduce 
      // the number of includes.
      InstructionBuffer instrBuffer_;
  };

  // TODO: Constant iterator version + integrate it in BCModule
  //       with 'instrs_beg' 'instrs_end' and 'instrs_back'
  //       For the constant iterator, maybe use a common template base.

  // An iterator to an instruction in a BCModule. 
  //
  // This wraps a BCModule* + an index because traditional vector iterators
  // cannot be used safely due to potential reallocations.
  class BCModule::instr_iterator {
    // Use a 32 bit unsigned int as index type to save a bit of space.
    using idx_type = std::uint32_t;
    public:
      using iterator_category = std::random_access_iterator_tag;
      using value_type = Instruction;
      using reference_type = Instruction&;
      using difference_type = idx_type;

      // Pre-increment
      instr_iterator& operator++();
      // Post-increment
      instr_iterator operator++(int);

      reference_type operator*() const;
      reference_type operator->() const; 

      friend bool operator==(instr_iterator lhs, instr_iterator rhs);
      friend bool operator!=(instr_iterator lhs, instr_iterator rhs);

    private:
      friend class BCModuleBuilder;

      reference_type& get();
      const reference_type& get() const;
      
      instr_iterator(BCModule& bcModule, idx_type idx);

      BCModule& bcModule_;
      idx_type idx_;
  };
}