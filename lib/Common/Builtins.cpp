//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Builtins.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/Builtins.hpp"
#include "Fox/Common/BuiltinID.hpp"
#include "Fox/Common/Errors.hpp"
#include "Fox/Common/Objects.hpp"
#include "Fox/Common/UTF8.hpp"
#include <iostream>

using namespace fox;

//----------------------------------------------------------------------------//
// BuiltinID
//----------------------------------------------------------------------------//

namespace {
  template<typename Rtr, typename ... Args>
  constexpr bool isReturnTypeVoid(Rtr(*)(Args...)) {
    return false;
  }

  template<typename ... Args>
  constexpr bool isReturnTypeVoid(void(*)(Args...)) {
    return true;
  }
}

bool fox::hasNonVoidReturnType(BuiltinID id) {
  switch (id) {
    #define BUILTIN(FUNC) \
    case BuiltinID::FUNC: \
      return !isReturnTypeVoid(builtin::FUNC);
    #include "Fox/Common/Builtins.def"
    default:
      fox_unreachable("unknown BuiltinID");
  }
}

const char* fox::to_string(BuiltinID id) {
  switch (id) {
    #define BUILTIN(FUNC) case BuiltinID::FUNC: return #FUNC;
    #include "Fox/Common/Builtins.def"
    default:
      fox_unreachable("unknown BuiltinID");
  }
}

std::ostream& fox::operator<<(std::ostream& os, BuiltinID id) {
  return os << to_string(id);
}

//----------------------------------------------------------------------------//
// Utils
//----------------------------------------------------------------------------//

std::string builtin::util::to_string(FoxInt value) {
  return std::to_string(value);
}

std::string builtin::util::to_string(FoxDouble value) {
  return std::to_string(value);
}

std::string builtin::util::to_string(bool value) {
  return (value ? "true" : "false");
}

std::string builtin::util::to_string(FoxChar value) {
  std::string dest;
  appendFoxChar(value, dest);
  return dest;
}


//----------------------------------------------------------------------------//
// Builtins
//----------------------------------------------------------------------------//

void builtin::printInt(FoxInt value) {
  std::cout << util::to_string(value);
}

void builtin::printBool(bool value) {
  std::cout << util::to_string(value);
}

void builtin::printChar(FoxChar ch) {
  std::cout << util::to_string(ch);
}

void builtin::printDouble(FoxDouble value) {
  std::cout << util::to_string(value);
}

void builtin::printString(StringObject* str) {
  assert(str && "String is Null!");
  std::cout << str->str();
}