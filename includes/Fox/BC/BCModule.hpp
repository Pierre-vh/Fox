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
#include <iosfwd>

namespace fox {
  class BCModule {
    public:
      BCModule() = default;
      BCModule(const BCModule&) = delete;
      BCModule& operator=(const BCModule&) = delete;

      /// \returns the number of instructions in the instruction buffer
      std::size_t numInstructions() const;

      /// \returns a reference to the instruction buffer
      InstructionVector& getInstructions();

      /// \returns a constant reference to the instruction buffer
      const InstructionVector& getInstructions() const;

      /// Dumps the module to 'out'
      void dumpModule(std::ostream& out) const;

      /// \returns the begin iterator for the instruction vector
      InstructionVector::iterator instrs_begin();
      /// \returns the end iterator for the instruction vector
      InstructionVector::iterator instrs_end();

    private:
      InstructionVector instrs_;
  };
}