//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Errors.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/Errors.hpp"
#include <cstdlib>
#include <iostream>

void fox::detail::fox_unreachable_internal(const char* message, 
                                           const char* file, unsigned line) {
  std::cerr << "(" << file << ", l:" << line 
            << ") UNREACHBLE INSTRUCTION EXECUTED: \"" 
            << message << "\"\n";
  abort();
}

void fox::detail::fox_unimpl_feat_internal(const char* message, 
                                           const char* file, unsigned line) {
  std::cerr << "(" << file << ", l:" << line 
            << ") UNIMPLEMENTED FEATURE: \"" 
            << message << "\"\n";
  abort();
}