////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Utils.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Defines some general utility functions & macros									
////------------------------------------------------------////

#pragma once

#include <type_traits>
#include <cassert>

namespace fox
{
	// convert enum to integer
	template<typename Enumeration>
	inline auto enumAsInt(Enumeration const value) -> typename std::underlying_type<Enumeration>::type
	{
		return static_cast<typename std::underlying_type<Enumeration>::type>(value);
	}

	[[ noreturn ]]
	void _fox_unreachable_internal(const char* message, const char* file, unsigned line);
}


// Quick macro to mark some piece of code as unreachable.
#ifndef fox_unreachable
	#define fox_unreachable(msg) ::fox::_fox_unreachable_internal(msg,__FILE__,__LINE__)
#endif