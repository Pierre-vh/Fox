#pragma once

// This file contains various defines used in the interpreter to alter its functions.
// Theses are mostly used for debugging.

// MOONSHOT_DETAILED_ERRORS : Error singleton will show the file and line of file where the error took place
#define MOONSHOT_DETAILED_ERRORS false


// Lexer :
#define LOG_PUSHEDTOKENS false			// log every pushed token
#define LOG_TOTALTOKENSCOUNT false		// log the total tokens count
