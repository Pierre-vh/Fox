////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Errors.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains some functions useful in 
// reporting critical errors. Such errors
// include "unreachable code executed" and "bad allocations"
////------------------------------------------------------////

#pragma once

#include <cassert>

namespace fox
{
	[[noreturn]]
	void _fox_unreachable_internal(const char* message, const char* file, unsigned line);
}


// Quick macro to mark some piece of code as unreachable.
#ifndef fox_unreachable
	#define fox_unreachable(msg) ::fox::_fox_unreachable_internal(msg,__FILE__,__LINE__)
#endif