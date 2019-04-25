//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BCFunction.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file declares the BCFunction class.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/BC/BCUtils.hpp"
#include "Fox/BC/Instruction.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"
#include <iosfwd>

namespace fox {
  class BCBuilder;

  class BCFunction {
    public:
      /// Creates a BCFunction
      /// \param id the ID of the function
      /// \param numParams the number of parameters the function takes
      BCFunction(std::size_t id, std::size_t numParams = 0);

      BCFunction(const BCFunction&) = delete;
      BCFunction& operator=(const BCFunction&) = delete;

      /// \returns the unique identifier of this function
      std::size_t getID() const {
        return id_;
      }
      
      /// \returns the number of instructions in the instruction buffer
      std::size_t numInstructions() const;

      /// \returns the number of parameters that this function takes
      std::size_t numParams() const {
        return numParams_;
      }

      /// Creates a bytecode builder for this function's instruction buffer.
      BCBuilder createBCBuilder();

      /// \returns a reference to the instruction buffer
      InstructionVector& getInstructions() {
        return instrs_;
      }

      /// \returns a constant reference to the instruction buffer
      const InstructionVector& getInstructions() const {
        return instrs_;
      }

      /// Dumps the module to 'out'
      void dump(std::ostream& out) const;

      /// \returns the begin iterator for the instruction buffer
      InstructionVector::iterator instrs_begin();
      /// \returns the begin iterator for the instruction buffer
      InstructionVector::const_iterator instrs_begin() const;
      /// \returns the end iterator for the instruction buffer
      InstructionVector::iterator instrs_end();
      /// \returns the end iterator for the instruction buffer
      InstructionVector::const_iterator instrs_end() const;

    private:
      InstructionVector instrs_;
      const std::size_t id_ = 0;
      const std::size_t numParams_ = 0;
  };
}