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
#include "Fox/Common/StableVectorIterator.hpp"
#include "llvm/ADT/SmallVector.h"
#include <iterator>
#include <functional>
#include <iosfwd>

namespace fox {
  class BCModule {
    public:
      using instr_iterator = StableVectorIterator<InstructionVector>;

      BCModule() = default;
      BCModule(const BCModule&) = delete;
      BCModule& operator=(const BCModule&) = delete;

      // Returns the number of instructions in the instruction buffer
      std::size_t numInstructions() const;

      // Returns a reference to the instruction buffer
      InstructionVector& getInstructions();

      // Returns a constant reference to the instruction buffer
      const InstructionVector& getInstructions() const;

      // Dumps the module to 'out'
      void dumpModule(std::ostream& out) const;

      // Returns an iterator to the first instruction in this module
      instr_iterator instrs_begin();
      // Returns a past-the-end iterator for this module's instruction buffer
      // This iterator is invalidated on insertion
      instr_iterator instrs_end();
      // Returns an iterator to the last instruction in this module
      instr_iterator instrs_last();

    private:
      friend class BCModuleBuilder;

      // Appends an in instruction to this Module, returning
      // an iterator to the pushed element
      instr_iterator addInstr(Instruction instr);

      InstructionVector instrBuffer_;
  };
}