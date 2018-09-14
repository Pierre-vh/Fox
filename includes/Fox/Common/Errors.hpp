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
	// For use with fox_unreachable only.
	[[noreturn]]
	void _fox_unreachable_internal(const char* message, const char* file, unsigned line);

	// Report a bad alloc error (when an allocator fails)
	// Currently this will print the error message to cerr
	// and throw std::bad_alloc
	void reportBadAlloc(const char* message);
}


// Quick macro to mark some piece of code as unreachable.
#ifndef fox_unreachable
	#define fox_unreachable(msg) ::fox::_fox_unreachable_internal(msg,__FILE__,__LINE__)
#endif