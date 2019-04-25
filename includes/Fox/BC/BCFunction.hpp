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
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/SmallVector.h"
#include <iosfwd>

namespace fox {
  class BCBuilder;

  class BCFunction {
    public:
      /// The map of parameters of the function that need to be
      /// copied after returning from the function.
      /// 0 = Does not need to be copied back after the function
      ///     has returned.
      /// 1 = needs to
      /// TODO: Find a better name for this
      using ParamCopyMap = llvm::BitVector;
      
      /// Creates a BCFunction that does not take any parameter
      /// \param id the ID of the function
      BCFunction(std::size_t id);

      /// Creates a BCFunction that takes parameters. 
      /// Information about parameters is stored in \p paramCopyMap
      /// \param id the ID of the function
      /// \param paramCopyMap the 'Param Copy Map' of this function
      BCFunction(std::size_t id, ParamCopyMap paramCopyMap);

      BCFunction(const BCFunction&) = delete;
      BCFunction& operator=(const BCFunction&) = delete;

      /// \returns the unique identifier of this function
      std::size_t getID() const;
      
      /// \returns the number of instructions in the instruction buffer
      std::size_t numInstructions() const;

      /// Creates a bytecode builder for this function's instruction buffer.
      BCBuilder createBCBuilder();

      /// \returns a reference to the instruction buffer
      InstructionVector& getInstructions();
      /// \returns a constant reference to the instruction buffer
      const InstructionVector& getInstructions() const;

      /// \returns the ParamCopyMap
      const ParamCopyMap& getParamCopyMap() const;

      /// \returns true if, after this function returns, we need to
      ///          copy some parameters back into the caller's stack.
      bool needsCopyAfterReturn() const;

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

      const ParamCopyMap paramCopyMap_;
      // Set to true if any bit in paramMap_ is set to true
      bool needsCopyAfterReturn_ = false;
  };
}