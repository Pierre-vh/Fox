////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Enums.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Enums.h"

using namespace Moonshot;

std::string Moonshot::getFromDict(const std::map<operation, std::string>& m, const operation& op)
{
	auto i = m.find(op);
	if (i != m.end())
		return i->second;
	return "";
}
