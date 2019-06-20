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
#include "Fox/BC/DebugInfo.hpp"
#include "Fox/BC/Instruction.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/Common/string_view.hpp"
#include "llvm/ADT/SmallVector.h"
#include <iosfwd>
#include <memory>

namespace fox {
  class BCBuilder;
  class DebugInfo;

  /// A Bytecode function, which can be either a function or a global variable's
  /// initializer.
  class BCFunction {
    public:
      /// Creates a BCFunction
      /// \param id the ID of the function
      BCFunction(func_id_t id) : id_(id) {}

      BCFunction(const BCFunction&) = delete;
      BCFunction& operator=(const BCFunction&) = delete;

      /// \returns the id of this function
      func_id_t getID() const {
        return id_;
      }
      
      /// \returns the number of instructions in the instruction buffer
      std::size_t numInstructions() const {
        return instrs_.size();
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

      /// Dumps this function to 'out'
      /// \param out the output stream
      /// \param title the title of the function. By default, "Function".
      /// The title is printed before the function's id. So in this case,
      /// if the function's ID is "0", it'd print "Function 0"
      void dump(std::ostream& out, string_view title = "Function") const;

      /// creates an instance of DebugInfo for this function
      /// \returns a reference to the instance created
      DebugInfo& createDebugInfo();
      
      /// Removes the debug info currently attached to this function
      void removeDebugInfo();

      /// \returns true if this function has a DebugInfo instance associated
      /// with it.
      bool hasDebugInfo() const;

      /// \returns the DebugInfo instance attached to this function 
      /// (can be null)
      DebugInfo* getDebugInfo();

      /// \returns the DebugInfo instance attached to this function 
      /// (can be null)
      const DebugInfo* getDebugInfo() const;

      /// \returns the begin iterator for the instruction buffer
      InstructionVector::iterator instrs_begin() {
        return instrs_.begin();
      }

      /// \returns the begin iterator for the instruction buffer
      InstructionVector::const_iterator instrs_begin() const {
        return instrs_.begin();
      }

      /// \returns the end iterator for the instruction buffer
      InstructionVector::iterator instrs_end() {
        return instrs_.end();
      }

      /// \returns the end iterator for the instruction buffer
      InstructionVector::const_iterator instrs_end() const {
        return instrs_.end();
      }

    private:
      /// The buffer of instructions
      InstructionVector instrs_;

      /// The ID of this function
      const func_id_t id_ = 0;

      /// The (optional) debug information for this function
      std::unique_ptr<DebugInfo> debugInfo_;
  };
}