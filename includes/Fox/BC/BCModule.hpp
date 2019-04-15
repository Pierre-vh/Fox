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
#include "Fox/Common/FoxTypes.hpp"
#include "llvm/ADT/SmallVector.h"
#include <memory>
#include <string>
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
      /// \returns a const reference to the functions vector
      const FunctionVector& getFunctions() const;

      /// Adds a new string constant into the BCModule.
      /// This is simply a push_back operation, it does not unique the constant.
      /// \param str the string to insert
      /// \returns the index of the newly inserted constant
      std::size_t addStringConstant(const std::string& str);
      /// \returns the int constant identified by \p idx
      std::string getStringConstant(std::size_t idx) const;
      /// \returns a view of the string constants vector
      ArrayRef<std::string> getStringConstants() const;
      
      /// Adds a new int constant into the BCModule.
      /// This is simply a push_back operation, it does not unique the constant.
      /// \param value the value to insert
      /// \returns the index of the newly inserted constant
      std::size_t addIntConstant(FoxInt value);
      /// \returns the int constant identified by \p idx
      FoxInt getIntConstant(std::size_t idx) const;
      /// \returns a view of the int constants vector
      ArrayRef<FoxInt> getIntConstants() const;

      /// Adds a new double constant into the BCModule.
      /// This is simply a push_back operation, it does not unique the constant.
      /// \param value the value to insert
      /// \returns the index of the newly inserted constant
      std::size_t addDoubleConstant(FoxDouble value);
      /// \returns the double constant identified by \p idx
      FoxDouble getDoubleConstant(std::size_t idx) const;
      /// \returns a view of the double constants vector
      ArrayRef<FoxDouble> getDoubleConstants() const;

      /// Dumps the module to 'out'
      void dump(std::ostream& out) const;

    private:
      FunctionVector functions_;

      // FIXME: The type of the values in the vectors should really be
      // constant, but SmallVector does not allow it for some reason.
      // Either change this to a std::vector, or find a workaround.
      SmallVector<std::string, 4> strConstants_;
      SmallVector<FoxInt, 4> intConstants_;
      SmallVector<FoxDouble, 4> doubleConstants_;
  };
}