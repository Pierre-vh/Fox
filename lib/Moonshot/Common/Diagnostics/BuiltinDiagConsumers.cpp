////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : BuiltinDiagConsumers.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "BuiltinDiagConsumers.hpp"
#include <iostream>

using namespace Moonshot;

void StdIoDiagConsumer::consume(const Diagnostic & diag)
{
	std::cout << "consuming.." << std::endl;
}
