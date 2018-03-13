////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : MemberAccess.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Tests for function declarations. Uses the test files located under /res/tests/funcdecl										
////------------------------------------------------------////

#pragma once

#include "../ITest.hpp"

namespace Moonshot::Test
{
	class MemberAccess : public ITest
	{
		public:
			MemberAccess() = default;

			virtual std::string getTestName() const override;
			virtual bool runTest(Context & context) override;
		private:
			bool testMembAccess(Context& context, const std::string& str);
	};
}
