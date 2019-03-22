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
#include <functional>
#include <iosfwd>

namespace fox {
  class BCModule {
    public:
      BCModule() = default;
      BCModule(const BCModule&) = delete;
      BCModule& operator=(const BCModule&) = delete;

      class instr_iterator;

      // Returns the number of instructions in the instruction buffer
      std::size_t numInstructions() const;

      // Returns a reference to the instruction buffer
      InstructionBuffer& getInstructionBuffer();

      // Returns a constant reference to the instruction buffer
      const InstructionBuffer& getInstructionBuffer() const;

      // Dumps the module to 'out'
      void dumpModule(std::ostream& out) const;

      // Erases all instructions in the range [beg, end)
      void erase(instr_iterator beg, instr_iterator end);

      // Removes the last instruction added to this module.
      void popInstr();

      instr_iterator instrs_begin();
      instr_iterator instrs_end();
      instr_iterator instrs_back();

    private:
      friend class BCModuleBuilder;

      // Appends an in instruction to this Module, returning
      // an iterator to the pushed element.
      instr_iterator addInstr(Instruction instr);

      InstructionBuffer instrBuffer_;
  };

  // An iterator to an instruction in a BCModule. 
  //
  // This wraps a BCModule* + an index because classic vector iterators
  // cannot be used safely due to potential reallocations 
  // (iterator invalidation)
  class BCModule::instr_iterator {
    using idx_type = std::size_t;
    public:
      using iterator_category = std::bidirectional_iterator_tag;
      using value_type = Instruction;
      using reference_type = Instruction&;
      using pointer_type = Instruction*;
      using difference_type = idx_type;

      instr_iterator& operator=(const instr_iterator& other);

      // Pre-increment
      instr_iterator& operator++();
      // Post-increment
      instr_iterator operator++(int);

      // Pre-decrement
      instr_iterator& operator--();
      // Post-decrement
      instr_iterator operator--(int);

      reference_type operator*() const;
      pointer_type operator->() const; 

      friend bool operator==(const instr_iterator& lhs, 
                             const instr_iterator& rhs);
      friend bool operator!=(const instr_iterator& lhs, 
                             const instr_iterator& rhs);

      friend bool operator<(const instr_iterator& lhs, 
                            const instr_iterator& rhs);
      friend bool operator>(const instr_iterator& lhs, 
                            const instr_iterator& rhs);

      // std::distance equivalent method for BCModule::instr_iterator.
      friend difference_type
      distance(instr_iterator first, instr_iterator last);

    private:
      InstructionBuffer::iterator toIBiterator() const;

      // Only the BCModule should be able to create these iterators.
      friend class BCModule;

      // Returns true if this iterator and the other
      // share the same module.
      bool usesSameModuleAs(const instr_iterator& other) const;

      reference_type& get() const;
      
      instr_iterator(BCModule& bcModule, idx_type idx);

      std::reference_wrapper<BCModule> bcModule_;
      idx_type idx_;
  };
}