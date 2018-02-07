////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : StringManipulator.h											
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
#include <iostream>
#include "../Types/Types.h"

namespace Moonshot::UTF8
{
	void skipBOM(std::string::iterator& it,std::string::iterator end);
	void append(std::string& str, const CharType& ch);
	class StringManipulator
	{
		public:
			StringManipulator();
			~StringManipulator();

			std::string getStr() const;
			void setStr(const std::string& str);

			std::string wcharToStr(const CharType& wc) const;

			void reset();
			void advance(const std::size_t& ind = 1);
			CharType currentChar() const;			

			CharType getChar(std::size_t ind) const;

			std::string substring(std::size_t beg, const std::size_t& leng) const;
			
			CharType peekFirst() const;
			CharType peekNext() const;
			CharType peekPrevious() const;
			CharType peekBack() const;

			std::size_t getSize() const;

			bool isAtEndOfStr() const;
		private:
			std::string str_;
			std::string::iterator iter_, end_, beg_;
	};
}

