//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See LICENSE.txt for license info.            
// File : Errors.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/Errors.hpp"
#include <cstdlib>
#include <new>  // bad_alloc
#include <iostream>

void fox::detail::fox_unreachable_internal(const char* message, const char* file, unsigned line) {
  std::cerr << "(" << file << ", l:" << line << ") UNREACHBLE INSTRUCTION EXECUTED: \"" << message << "\"\n";
  abort();
}

void fox::detail::fox_unimpl_feat_internal(const char* message, const char* file, unsigned line) {
  std::cerr << "(" << file << ", l:" << line << ") UNIMPLEMENTED FEATURE: \"" << message << "\"\n";
  abort();
}

void fox::reportBadAlloc(const char* message) {
  std::cerr << "bad_alloc: " << message << '\n';
  throw std::bad_alloc();
}
