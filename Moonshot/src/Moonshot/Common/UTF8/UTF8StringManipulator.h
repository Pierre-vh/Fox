////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : UTF8StringManipulator.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Implements a UTF-8 string manipulator based on the UTFCPP library.
////------------------------------------------------------////

#pragma once

#include <iterator>
#include <iostream>
#include "../Types/Types.h"
#include "../External/utfcpp/utf8.h"

namespace Moonshot
{
	class UTF8StringManipulator
	{
		public:
			UTF8StringManipulator();
			~UTF8StringManipulator();

			std::string getStr() const;
			void setStr(const std::string& str);

			void reset();
			void advance(const std::size_t& ind = 1);
			CharType currentChar();			

			CharType getChar(std::size_t ind) const;

			void append(std::string& str, const CharType& ch) const;

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

