////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IStringManipulator.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Defines the String manipulator Abstract class (interface).
// It's the base class of every String manipulation class.
////------------------------------------------------------////

#pragma once

#include <string>

namespace Moonshot
{
	class IStringManipulator
	{
		public:
			virtual void setStr(const std::string& str);	// set the string to be manipulated, and resets the cursor.
			virtual std::string getStr() const;				// returns str_

			virtual void reset() = 0;							// Resets the cursor back to 0 (start of string)
			virtual wchar_t next() = 0;							// Returns the current char and set the cursor to the next codepoint if possible;

			virtual wchar_t peekNext() = 0;				// Peeks the next character, doesn't move the cursor
			virtual wchar_t peekPrevious() = 0;			// Peeks the previous character, doesn't move the cursor
		
			virtual std::size_t getSize() const = 0;			// returns str_ CORRECT size (depending on it's encoding)
		
			virtual bool isAtEndOfStr() const = 0;
		protected:
			std::string str_;
	};
}
