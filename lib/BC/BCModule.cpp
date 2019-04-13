//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BCModule.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BC/BCModule.hpp"
#include "llvm/ADT/ArrayRef.h"

using namespace fox;

std::size_t BCModule::numFunctions() const {
  return functions_.size();
}

BCFunction& BCModule::createFunction() {
  functions_.push_back(std::make_unique<BCFunction>(numFunctions()));
  return *functions_.back();
}

BCFunction& BCModule::getFunction(std::size_t idx) {
  assert((idx < numFunctions()) && "out of range");
  return *functions_[idx];
}

const BCFunction& fox::BCModule::getFunction(std::size_t idx) const {
  assert((idx < numFunctions()) && "out of range");
  return *functions_[idx];
}

BCModule::FunctionVector& BCModule::getFunctions() {
  return functions_;
}

const BCModule::FunctionVector& BCModule::getFunctions() const {
  return functions_;
}

void BCModule::dump(std::ostream& out) const {
  for(auto& fn : functions_)
    fn->dump(out);
}
