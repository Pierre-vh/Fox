//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Opcode.cpp                    
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/VM/Opcode.hpp"

static constexpr const char* const opcodeStrings[] = {
  #define INSTR(Op) #Op,
  #include "Fox/VM/Instructions.def"
};

const char* fox::toString(Opcode op) {
  if(isLegalOpcode(op))
    return opcodeStrings[static_cast<std::uint8_t>(op)];
  return nullptr;
}
