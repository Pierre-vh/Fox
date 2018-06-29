////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Utils.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Utils.hpp"
#include <cstdlib>
#include <iostream>

void fox::_fox_unreachable_internal(const char* message, const char* file, const unsigned& line)
{
	std::cerr << "(" << file << ", l:" << line << ") UNREACHBLE INSTRUCTION EXECUTED: \"" << message << "\"\n";
	abort();
}