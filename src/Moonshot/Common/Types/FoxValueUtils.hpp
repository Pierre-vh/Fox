////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : FoxValueUtils.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This class defines various utility functions for FoxValues
////------------------------------------------------------////

#pragma once
#include "Types.hpp"

namespace Moonshot::FValUtils
{
	std::string dumpFVal(const FoxValue& fv);
	std::string getTypenameForIndex(const std::size_t& ind);
}