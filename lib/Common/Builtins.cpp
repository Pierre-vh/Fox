//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Builtins.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/Builtins.hpp"
#include "Fox/Common/Errors.hpp"
#include <iostream>

using namespace fox;

const char* fox::to_string(BuiltinID id) {
  switch (id) {
    case BuiltinID::invalid: return "<invalid>";
    #define BUILTIN(FUNC, FOX) case BuiltinID::FUNC: return #FUNC;
    #include "Fox/Common/Builtins.def"
    default:
      fox_unreachable("unknown BuiltinID");
  }
}

//----------------------------------------------------------------------------//
// Builtin implementations
//----------------------------------------------------------------------------//

void builtin::printInt(FoxInt value) {
  std::cout << value;
}

void builtin::printBool(bool value) {
  std::cout << (value ? "true" : "false");
}

