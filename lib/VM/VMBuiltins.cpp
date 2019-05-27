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

using namespace fox;

StringObject* builtin::charToString(VM& vm, FoxChar value) {
  return vm.newStringObject(util::toString(value));
}

StringObject* builtin::intToString(VM& vm, FoxInt value) {
  return vm.newStringObject(util::toString(value));
}

StringObject* builtin::doubleToString(VM& vm, FoxDouble value) {
  return vm.newStringObject(util::toString(value));
}

StringObject* builtin::boolToString(VM& vm, bool value) {
  return vm.newStringObject(util::toString(value));
}

StringObject* builtin::strConcat(VM& vm, StringObject* lhs, StringObject* rhs) {
  assert(lhs && "LHS is null");
  assert(rhs && "RHS is null");
  return vm.newStringObject(lhs->str() + rhs->str());
}

StringObject* builtin::charConcat(VM& vm, FoxChar lhs, FoxChar rhs) {
  // TODO: Can this be improved?
  std::string str;
  appendFoxChar(lhs, str);
  appendFoxChar(rhs, str);
  StringObject* result = vm.newStringObject(str);
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

FoxChar builtin::getChar(StringObject* str, FoxInt n) {
  assert(str && "string is null");
  assert((n >= 0) && (n < str->length()) && "out of range index");
  return str->getChar(static_cast<std::size_t>(n));
}

void builtin::arrAppend(ArrayObject* arr, FoxAny elem) {
  assert(arr && "array is null");
  arr->append(elem);
}
