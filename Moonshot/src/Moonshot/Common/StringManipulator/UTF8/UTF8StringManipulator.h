////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : UTF8StringManipulator.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Implements a UTF-8 string manipulator based on the UTFCPP library.
////------------------------------------------------------////

#pragma once

#include "../IStringManipulator.h"
#include <iterator>
#include <iostream>
#include "../../External/utfcpp/utf8.h"

namespace Moonshot
{
	class UTF8StringManipulator : public IStringManipulator
	{
		public:
			UTF8StringManipulator();
			~UTF8StringManipulator();


			virtual void setStr(const std::string& str) override;

			virtual void reset() override;
			virtual void advance(const std::size_t& ind = 1) override;
			virtual CharType currentChar() override;			

			virtual CharType getChar(std::size_t ind) const override;

			virtual void append(std::string& str, const CharType& ch) const override;

			virtual std::string substring(std::size_t beg, const std::size_t& leng) const;
			
			virtual CharType peekFirst() override;
			virtual CharType peekNext() override;
			virtual CharType peekPrevious() override;
			virtual CharType peekBack() override;

			virtual std::size_t getSize() const override;

			virtual bool isAtEndOfStr() const override;
		private:
			std::string::iterator iter_;
	};
}

