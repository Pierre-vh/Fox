////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : U8StrManipTest.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Tests for the StringManipulator.
// Uses test file located under res/tests/utf8
////------------------------------------------------------////

#pragma once

#include "../ITest.hpp"
#include "../../Common/UTF8/StringManipulator.hpp"
#include <cwctype>		// std::iswspace

namespace Moonshot
{
	class U8StrManipTest : public ITest
	{
		public:
			U8StrManipTest() = default;
			~U8StrManipTest();

			virtual std::string getTestName() const override;
			virtual bool runTest(Context & context) override;

		private:
			bool testStr(Context& context, const std::string& str, unsigned int explinecount, unsigned int expcharcount, unsigned int expspacecount); // Tests a string with X expected lines/chars/spaces
	};
}
