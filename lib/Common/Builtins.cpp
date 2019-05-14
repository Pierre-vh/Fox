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

const char* fox::to_string(BuiltinID id) {
  switch (id) {
    #define BUILTIN(FUNC, FOX) case BuiltinID::FUNC: return #FUNC;
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

void builtin::util::toString(std::string& dest, FoxInt value) {
  dest = std::to_string(value);
}

void builtin::util::toString(std::string& dest, FoxDouble value) {
  dest = std::to_string(value);
}

void builtin::util::toString(std::string& dest, bool value) {
  dest = (value ? "true" : "false");
}

void builtin::util::toString(std::string& dest, FoxChar value) {
  appendFoxChar(value, dest);
}


//----------------------------------------------------------------------------//
// Builtins
//----------------------------------------------------------------------------//

void builtin::printInt(FoxInt value) {
  std::string str;
  util::toString(str, value);
  std::cout << str;
}

void builtin::printBool(bool value) {
  std::string str;
  util::toString(str, value);
  std::cout << str;
}

void builtin::printChar(FoxChar ch) {
  std::string str;
  util::toString(str, ch);
  std::cout << str;
}

void builtin::printDouble(FoxDouble value) {
  std::string str;
  util::toString(str, value);
  std::cout << str;
}

void builtin::printString(StringObject* str) {
  assert(str && "String is Null!");
  std::cout << str->str();
}