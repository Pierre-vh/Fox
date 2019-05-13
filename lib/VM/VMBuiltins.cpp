//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : VMBuiltins.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// Implementation of Builtins that need the Fox VM to work.
//----------------------------------------------------------------------------//

#include "Fox/Common/Builtins.hpp"
#include "Fox/Common/Objects.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/UTF8.hpp"
#include "Fox/VM/VM.hpp"
#include <string>
#include <sstream>

using namespace fox;

StringObject* builtin::charToString(VM& vm, FoxChar value) {
  StringObject* result = vm.newStringObject();
  /// FIXME: Is this optimal?
  appendFoxChar(value, result->str());
  return result;
}

StringObject* builtin::intToString(VM& vm, FoxInt value) {
  std::stringstream ss;
  ss << value;
  return vm.newStringObject(ss.str());
}

StringObject* builtin::doubleToString(VM& vm, FoxDouble value) {
  return vm.newStringObject(std::to_string(value));
}

StringObject* builtin::boolToString(VM& vm, bool value) {
  return vm.newStringObject(value ? "true" : "false");
}


StringObject* builtin::strConcat(VM& vm, StringObject* lhs, StringObject* rhs) {
  assert(lhs && "LHS is null");
  assert(rhs && "RHS is null");
  StringObject* result = vm.newStringObject();
  std::string& str = result->str();
  str = lhs->str() + rhs->str();
  return result;
}

StringObject* builtin::charConcat(VM& vm, FoxChar lhs, FoxChar rhs) {
  StringObject* result = vm.newStringObject();
  result->append(lhs);
  result->append(rhs);
  return result;
}

FoxInt builtin::strLength(StringObject* str) {
  assert(str && "String is null");
  return str->length();
}

FoxInt builtin::strNumBytes(StringObject* str) {
  assert(str && "String is null");
  return str->numBytes();
}
