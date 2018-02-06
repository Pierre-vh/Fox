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
			virtual wchar_t next() override;
			virtual wchar_t peekNext() override;
			virtual wchar_t peekPrevious() override;
			virtual std::size_t getSize() const override;

			virtual bool isAtEndOfStr() const override;
		private:
			std::string::iterator iter_;			
	};
}

