////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Flags.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "FlagsManager.hpp"
#include "Moonshot/Common//Exceptions/Exceptions.hpp"
using namespace Moonshot;

#define UNKNOWN_KEY_EXCEPTION "Enum value does not exists in map. This can happen if you add a new enum value without using the .def files!"

bool FlagsManager::isSet(const FlagID& fid) const
{
	if (existsInMap(flags_, fid))
		return flags_.find(fid)->second;
	else
		throw std::out_of_range(UNKNOWN_KEY_EXCEPTION);
}

void FlagsManager::set(const FlagID& fid)
{
	if (existsInMap(flags_, fid))
		flags_[fid] = true;
	else
		throw std::out_of_range(UNKNOWN_KEY_EXCEPTION);
}

void FlagsManager::unset(const FlagID& fid)
{
	if (existsInMap(flags_, fid))
		flags_[fid] = false;
	else
		throw std::out_of_range(UNKNOWN_KEY_EXCEPTION);
}