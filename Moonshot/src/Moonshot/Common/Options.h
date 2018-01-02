////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Options.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares various "Options" macro.											
////------------------------------------------------------////

#pragma once

// This file contains various defines used in the interpreter to alter its functions.
// Theses are mostly used for debugging.

// MOONSHOT_DETAILED_ERRORS : if true, the error singleton will show the file and line of file where the error took place
// Useful for debugging, but in most cases you don't need it.
#define MOONSHOT_DETAILED_ERRORS false

// Lexer :
#define LOG_PUSHEDTOKENS false			// log every pushed token
#define LOG_TOTALTOKENSCOUNT false		// log the total tokens count

// Symbols table
#define LOG_IMPLICIT_CASTS false		// log when a value had to be cast before being stored in memory