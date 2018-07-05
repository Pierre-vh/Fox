////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : StringManipulator.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Implements a UTF-8 string manipulator based on the UTFCPP library.
// It always work with a pointer to the string to avoid copies.
//
// How it works :
//
// # = Current iterator iter_ position. It is always positioned at the end of the previous CP, and at the beginning of the current CP
// CP = One codepoint. It's one or more bytes in the std::string.
//		
//													  getCurrentChar()
//															|->
//											  peekPrevious()|	peekNext()
//													|->   |		|->
//							 	- - --------------------------------------------------------- - -
//	str_(input string)		 		|	CP	|	CP	|	CP	#	CP	|	CP	|	CP	|	CP	|
//							 	- - --------------------------------------------------------- - -
//
//
////------------------------------------------------------////

#pragma once

#include "Fox/Common/Typedefs.hpp"
#include <string>

namespace fox
{
	class StringManipulator
	{
		public:
			// Default ctor
			StringManipulator() = default;
			StringManipulator(const std::string* str);

			/*
				STRING GETTERS/SETTERS
			*/	

			// Returns a pointer to the string
			const std::string* getStr() const;	

			// Set this SM's source to a the pointer str
			void setStr(const std::string* str);

			/*
				ITERATOR MANIPULATION
			*/
			void reset();
			void advance(const std::size_t& ind = 1);
			void goBack(const std::size_t& ind = 1);

			/*
				GET THE CURRENT CHARACTER
			*/

			// Get the current codepoint
			CharType getCurrentChar() const;

			// Get a codepoint at a precise location
			CharType getChar(std::size_t ind) const;
			
			/*
				PEEK
			*/

			CharType peekFirst() const;
			CharType peekNext() const;
			CharType peekPrevious() const;
			CharType peekBack() const;

			/*
				UTILS & OTHERS
			*/

			// Extract a substring
			std::string substring(std::size_t beg, std::size_t leng) const;

			// Return the number of codepoints in the string
			std::size_t getSizeInCodepoints() const;

			// Returns the number of bytes in the string
			std::size_t getSizeInBytes() const;

			// Checks if the stringmanipulator has reached the end of the string
			bool eof() const;

			// Returns the index of the current character in codepoints
			// DO NOT MIX THIS WITH std::string::operator[] AND STRING OPERATIONS!
			std::size_t getIndexInCodepoints() const;

			// This uses std::distance to calculate the index at which the current codepoint begins in BYTES
			// You can use this with std::string::operator[] to retrieve the first byte of the codepoint.
			std::size_t getIndexInBytes() const;

			/*
				STATIC METHODS
			*/

			// Convert a CharType to a utf8 encoded string
			static std::string charToStr(CharType wc);

			// Removes the BOM from a str
			static void removeBOM(std::string& str);

			// Given 2 iterators, places the iterator it at the beginning of the first codepoint, ignoring the Byte order mark
			template<typename it_type>
			static void skipBOM(it_type& it, it_type end)
			{
				if (utf8::starts_with_bom(it, end))
					utf8::next(it, end);
			}

			// Appends a CharType to a std::string.
			static void append(std::string& str, CharType ch);
		private:
			// Get a reference to the string stored.
			const std::string& str() const;

			// The string currently stored
			const std::string* raw_str_;

			// Iterators
			std::string::const_iterator iter_, end_, beg_;
	};
}

