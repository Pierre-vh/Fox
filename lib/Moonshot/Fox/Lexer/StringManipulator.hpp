////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : StringManipulator.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Implements a UTF-8 string manipulator based on the UTFCPP library.
//
// How it works :
//
// # = Current iterator iter_ position. It is always positioned at the end of the previous CP, and at the beginning of the current CP
// CP = One codepoint. It's one or more bytes in the std::string.
//		
//													  currentChar()
//															|
//											  peekPrevious()|	peekNext()
//													|	    |		|
//							>	- - --------------------------------------------------------- - -
//	str_(input string)		>		|	CP	|	CP	|	CP	#	CP	|	CP	|	CP	|	CP	|
//							>	- - --------------------------------------------------------- - -
//
//
////------------------------------------------------------////

#pragma once

#include <iterator>
#include <variant>
#include "Moonshot/Common/Types/Types.hpp"

namespace Moonshot::UTF8
{

	class StringManipulator
	{
		public:
			// Default ctor
			StringManipulator() = default;

			// Returns a copy of the internal string
			std::string getStrCpy() const;			

			// Returns a pointer to the string (no copy performed)
			const std::string* getStrPtr() const;	

			// Set this SM's source to a copy of str
			void setStr(const std::string& str);

			// Set this SM's source to a the pointer str
			void setStr(std::string* str);

			// Returns true if this SM uses a std::string* as source.
			bool isUsingAPointer() const;

			// Returns true if this SM uses a copy of a string as source.
			bool isUsingACopy() const;

			// Convert a CharType to a utf8 encoded string
			static std::string wcharToStr(const CharType& wc);

			// Removes the BOM from a str
			static void removeBOM(std::string& str);

			// Given 2 iterators, places the iterator it at the beginning of the first codepoint, ignoring the Byte order mark
			static void skipBOM(std::string::iterator& it, std::string::iterator end);

			// Appends a CharType to a std::string.
			static void append(std::string& str, const CharType& ch);

			// Returns the index of the current character in codepoints. 
			// THIS INDEX DIFFERS FROM THE STD::STRING'S OPERATOR [] EXPECTED INDEX.
			// TO GET THE CODEPOINT,USE A STRING MANIPULATOR's ADVANCE METHOD.
			std::size_t indexOfCurrentCharacter() const;

			// Reset the iterators
			void reset();

			// Advance (ind) codepoints
			void advance(const std::size_t& ind = 1);

			// Go back (ind) codepoints
			void goBack(const std::size_t& ind = 1);

			// Get the current codepoint
			CharType currentChar() const;			

			// Get a codepoint at a precise location
			CharType getChar(std::size_t ind) const;

			// Extract a substring
			std::string substring(std::size_t beg, const std::size_t& leng) const;
			
			// Peeking 
			CharType peekFirst() const;
			CharType peekNext() const;
			CharType peekPrevious() const;
			CharType peekBack() const;

			// Return the number of codepoints in string
			std::size_t getSize() const;

			// Checks if the stringmanipulator has reached the end of the string
			bool isAtEndOfStr() const;
		private:
			std::string& str();
			const std::string& str() const;

			std::variant<std::string,std::string*> data_;
			std::string::iterator iter_, end_, beg_;
	};
}

