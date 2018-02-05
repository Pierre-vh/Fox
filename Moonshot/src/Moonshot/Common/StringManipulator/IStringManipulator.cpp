////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IStringManipulator.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "IStringManipulator.h"

using namespace Moonshot;

void IStringManipulator::setStr(const std::string & str)
{
	str_ = str;
}

std::string IStringManipulator::getStr() const
{
	return str_;
}
