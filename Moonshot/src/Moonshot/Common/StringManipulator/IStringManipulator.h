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
#include "../Types/Types.h"

namespace Moonshot
{
	class IStringManipulator
	{
		public:
			virtual void setStr(const std::string& str);			// set the string to be manipulated, and resets the cursor.
			virtual std::string getStr() const;						// returns str_

			virtual void reset() = 0;								// Resets the cursor back to 0 (start of string)
			virtual void advance(const std::size_t& ind = 1) = 0;	// advances the cursor by X codepoint. if no argument is passed, just advance forward 1 cp.
			virtual CharType currentChar() = 0;						// returns the current character the cursor points to

			virtual CharType getChar(std::size_t ind) const = 0;				// returns the nth char without modifying the cursor.

			virtual void append(std::string& str, const CharType& ch) const = 0; // appends the character at the end of str_. Used to add CharType to strings. Doesn't modify iter or interacts with the str_ whatsoever.
			
			virtual std::string substring(std::size_t beg, const std::size_t& leng) const = 0; 
			
			virtual CharType peekFirst() = 0;
			virtual CharType peekBack() = 0;					// Returns the last character of the string without moving the cursor
			virtual CharType peekNext() = 0;					// Peeks the next character, doesn't move the cursor
			virtual CharType peekPrevious() = 0;				// Peeks the previous character, doesn't move the cursor
		
			virtual std::size_t getSize() const = 0;				// returns str_ size (number of codepoints, depending on it's encoding)
		
			virtual bool isAtEndOfStr() const = 0;
		protected:
			std::string str_;
	};
}
