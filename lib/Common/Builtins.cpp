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
#include <iomanip>
#include <sstream>

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

std::string builtin::detail::foxDoubleTostring(FoxDouble value) { 
  static std::size_t foxDoublePrec = 
    std::numeric_limits<FoxDouble>::max_digits10;
  // FIXME: Can this be done better?
  std::stringstream ss;
  ss << std::setprecision(foxDoublePrec) << value;
  return ss.str();
}

//----------------------------------------------------------------------------//
// Builtins
//
// FIXME: printXX builtins are implemented differently than XXToString functions
// maybe this can lead to inconsistencies?
//----------------------------------------------------------------------------//

static std::size_t foxDoubleMaxPrecision =
  std::numeric_limits<FoxDouble>::max_digits10;

void builtin::printInt(FoxInt value) {
  std::cout << value;
}

void builtin::printBool(bool value) {
  std::cout << (value ? "true" : "false");
}

void builtin::printChar(FoxChar ch) {
  // FIXME: This isn't very efficient
  std::string dest;
  appendFoxChar(ch, dest);
  std::cout << dest;
}

void builtin::printDouble(FoxDouble value) {
  std::cout << detail::foxDoubleTostring(value);
}

void builtin::printString(StringObject* str) {
  assert(str && "String is Null!");
  std::cout << str->str();
}


