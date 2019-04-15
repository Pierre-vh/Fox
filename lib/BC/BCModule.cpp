//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : BCModule.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BC/BCModule.hpp"
#include "Fox/Common/QuotedString.hpp"
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
  assert((idx < numFunctions())&& "out of range");
  return *functions_[idx];
}

const BCFunction& BCModule::getFunction(std::size_t idx) const {
  assert((idx < numFunctions())&& "out of range");
  return *functions_[idx];
}

BCModule::FunctionVector& BCModule::getFunctions() {
  return functions_;
}

const BCModule::FunctionVector& BCModule::getFunctions() const {
  return functions_;
}

std::size_t BCModule::addStringConstant(const std::string& str) {
  std::size_t idx = strConstants_.size();
  strConstants_.push_back(str);
  return idx;
}

std::string BCModule::getStringConstant(std::size_t idx) const {
  assert((idx < strConstants_.size()) && "out-of-range");
  return strConstants_[idx];
}

ArrayRef<std::string> BCModule::getStringConstants() const {
  return strConstants_;
}

std::size_t BCModule::addIntConstant(FoxInt value) {
  std::size_t idx = intConstants_.size();
  intConstants_.push_back(value);
  return idx;
}

FoxInt BCModule::getIntConstant(std::size_t idx) const {
  assert((idx < intConstants_.size()) && "out-of-range");
  return intConstants_[idx];
}

ArrayRef<FoxInt> BCModule::getIntConstants() const {
  return intConstants_;
}

std::size_t BCModule::addDoubleConstant(FoxDouble value) {
  std::size_t idx = doubleConstants_.size();
  doubleConstants_.push_back(value);
  return idx;
}

FoxDouble BCModule::getDoubleConstant(std::size_t idx) const {
  assert((idx < doubleConstants_.size()) && "out-of-range");
  return doubleConstants_[idx];
}

ArrayRef<FoxDouble> BCModule::getDoubleConstants() const {
  return doubleConstants_;
}

void BCModule::dump(std::ostream& out) const {
  out << "----Constants----\n";

  if(std::size_t size = intConstants_.size()) {
    out << "  [Integers: " << size << " constants]\n";
    for (std::size_t idx = 0; idx < size; ++idx) 
      out << "    " << idx << "\t| " << intConstants_[idx] << '\n';
  }
  else 
    out << "  [No Integer Constants]\n";

  if(std::size_t size = doubleConstants_.size()) {
    out << "  [Floating-Point: " << size << " constants]\n";
    for (std::size_t idx = 0; idx < size; ++idx) 
      out << "    " << idx << "\t| " << doubleConstants_[idx] << '\n';
  }
  else 
    out << "  [No Floating-Point Constants]\n";

  if(std::size_t size = strConstants_.size()) {
    out << "  [Strings: " << size << "]\n";
    for (std::size_t idx = 0; idx < size; ++idx) {
      out << "    " << idx << "\t| ";
      printQuotedString(strConstants_[idx], out, '"');
      out << '\n';
    }
  }
  else 
    out << "  [No String Constants]\n";

  out << "----Functions----\n";
  for (auto& fn : functions_) {
    // Print a newline before each function dump so it's more readable.
    out << '\n';
    fn->dump(out);
  }
}
