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
#include "Fox/Common/FoxTypes.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/Common/string_view.hpp"
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

      /// \returns the number of global variables in the module
      std::size_t numGlobals() const;

      /// Creates a new function (that will be stored in this module)
      /// \returns a reference to the created function
      BCFunction& createFunction();

      /// Creates a new global variable (that will be stored in this module)
      /// \returns a reference to the created function responsible for 
      ///          initializing this global variable.
      /// NOTE: Global initializers shouldn't call setGlobal manually.
      /// They should simply return the value that the global should have.
      /// e.g. to init it to 0, the initializer should do
      /// \verbatim
      ///   StoreSmallInt 0 0
      ///   Ret 0
      /// \endverbatim
      /// Nothing more, nothing less.
      BCFunction& createGlobalVariable();

      /// \returns a reference to the function in this module with ID \p idx
      BCFunction& getFunction(std::size_t idx);
      /// \returns a const reference to the function in this module 
      ///          with ID \p idx
      const BCFunction& getFunction(std::size_t idx) const;

      /// \returns a reference to the initializer function for the 
      ///          global with ID \p idx
      BCFunction& getGlobalVarInitializer(std::size_t idx);
      /// \returns a const reference to the initializer function for the global
      ///          with ID \p idx
      const BCFunction& getGlobalVarInitializer(std::size_t idx) const;

      /// \returns a reference to the functions vector
      FunctionVector& getFunctions();
      /// \returns a const reference to the functions vector
      const FunctionVector& getFunctions() const;

      /// \returns a reference to the global variable initializers vector
      FunctionVector& getGlobalVarInitializers();
      /// \returns a const reference to the global variable initializers vector
      const FunctionVector& getGlobalVarInitializers() const;

      /// Adds a new string constant into the BCModule.
      /// This is simply a push_back operation, it does not unique the constant.
      /// \param str the string to insert. it'll be copied in the vector.
      /// \returns the index of the newly inserted constant
      std::size_t addStringConstant(string_view str);
      /// \returns the string constant identified by \p idx
      const std::string& getStringConstant(std::size_t idx) const;
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

      /// \returns the entry point of this BCModule
      BCFunction* getEntryPoint();
      /// \returns the entry point of this BCModule
      const BCFunction* getEntryPoint() const;

      /// sets the entry point of this module to \p func. Cannot be changed
      /// once set.
      void setEntryPoint(BCFunction& func);

      /// \returns true if the module is completely empty
      bool empty() const;

      /// \returns true if the module does not contain any constant
      bool empty_constants() const;

      /// Dumps the module to 'out'
      void dump(std::ostream& out) const;

      /// TODO
      /// Checks BCModule invariants, printing errors to \p out
      /// \returns true if the module is valid
      /// bool verify(std::ostream& out); 

    private:
      /// Functions
      FunctionVector functions_;
      /// Global Variable Initialization Functions
      FunctionVector globalVarInitializers_;
      /// Entry function
      BCFunction* entryPoint_ = nullptr;

      SmallVector<std::string, 4> strConstants_;
      SmallVector<FoxInt, 4> intConstants_;
      SmallVector<FoxDouble, 4> doubleConstants_;
  };
}