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
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/VM/VM.hpp"

using namespace fox;

StringObject* builtin::charToString(VM& vm, FoxChar value) {
  return vm.newStringObject(util::to_string(value));
}

StringObject* builtin::intToString(VM& vm, FoxInt value) {
  return vm.newStringObject(util::to_string(value));
}

StringObject* builtin::doubleToString(VM& vm, FoxDouble value) {
  return vm.newStringObject(util::to_string(value));
}

StringObject* builtin::boolToString(VM& vm, bool value) {
  return vm.newStringObject(util::to_string(value));
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

// Checks that \p idx is not negative or >= \p size
// If it is, emits a diagnostic and returns false.
static bool checkSubscript(VM& vm, std::size_t size, FoxInt idx) {
  if (idx < 0) {
    vm.diagnose(DiagID::runtime_subscript_negative_index)
      .addArg(idx);
    return false;
  }
  if (idx >= size) {
    vm.diagnose(DiagID::runtime_subscript_out_of_range)
      .addArg(size).addArg(idx);
    return false;
  }
  return true;
}

FoxChar builtin::getChar(VM& vm, StringObject* str, FoxInt n) {
  assert(str && "string is null");
  if(!checkSubscript(vm, str->length(), n))
    return 0;
  // Do the subscript
  return str->getChar(static_cast<std::size_t>(n));
}

void builtin::arrAppend(ArrayObject* arr, FoxAny elem) {
  assert(arr && "array is null");
  arr->append(elem);
}

FoxInt builtin::arrSize(ArrayObject* arr) {
  assert(arr && "array is null");
  return arr->size();
}

FoxAny builtin::arrGet(VM& vm, ArrayObject* arr, FoxInt n) {
  if(!checkSubscript(vm, arr->size(), n))
    return FoxAny();
  return (*arr)[n];
}

FoxAny builtin::arrSet(VM& vm, ArrayObject* arr, FoxInt n, FoxAny val) {
  if(!checkSubscript(vm, arr->size(), n))
    return FoxAny();
  (*arr)[n] = val;
  return val;
}

/// Checks if \p arr is empty, if it is, emit a 
/// runtime_cannot_call_on_empty_array diagnostic with \p fnName as argument.
/// \returns true if the array was empty
static bool 
checkIfEmptyArrayForCall(VM& vm, ArrayObject* arr, string_view fnName) {
  if(arr->size()) return false;
  vm.diagnose(DiagID::runtime_cannot_call_on_empty_array).addArg(fnName);
  return true;
}

void builtin::arrPop(VM& vm, ArrayObject* arr) {
  assert(arr && "array is null");
  if(checkIfEmptyArrayForCall(vm, arr, "array.pop")) return;
  arr->pop();
}

FoxAny builtin::arrFront(VM& vm, ArrayObject* arr) {
  assert(arr && "array is null");
  if(checkIfEmptyArrayForCall(vm, arr, "array.front")) return FoxAny();
  return arr->front();
}

FoxAny builtin::arrBack(VM& vm, ArrayObject* arr) {
  assert(arr && "array is null");
  if(checkIfEmptyArrayForCall(vm, arr, "array.back")) return FoxAny();
  return arr->back();
}

void builtin::arrReset(ArrayObject * arr) {
  assert(arr && "array is null");
  arr->reset();
}
