//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BCModule.hpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  This file contains the BCModule class, which represents a VM program
//  that can be executed by the Fox VM. It contains functions, constants and
//  other contextual information.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/BC/BCUtils.hpp"
#include "Fox/BC/BCFunction.hpp"
#include "Fox/BC/Instruction.hpp"
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/SmallVector.h"
#include <memory>
#include <iosfwd>

namespace fox {
  class BCFunction;
  class BCModule {
    public:
      using FunctionVector = SmallVector<std::unique_ptr<BCFunction>, 4>;

      BCModule() = default;
      BCModule(const BCModule&) = delete;
      BCModule& operator=(const BCModule&) = delete;

      /// \returns the number of functions in the module
      std::size_t numFunctions() const;

      /// Creates a new function stored in this module.
      /// \returns a reference to the created function
      BCFunction& createFunction();

      /// \returns a reference to the function in this module with ID \p idx
      BCFunction& getFunction(std::size_t idx);
      /// \returns a const reference to the function in this module 
      ///          with ID \p idx
      const BCFunction& getFunction(std::size_t idx) const;

      /// \returns a reference to the functions vector
      FunctionVector& getFunctions();
      /// \returns a  const reference to the functions vector
      const FunctionVector& getFunctions() const;

      /// Dumps the module to 'out'
      void dump(std::ostream& out) const;

    private:
      FunctionVector functions_;
  };
}