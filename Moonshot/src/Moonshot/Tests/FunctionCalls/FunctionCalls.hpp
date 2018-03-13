////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : FunctionsCalls.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Tests for function declarations. Uses the test files located under /res/tests/funcdecl										
////------------------------------------------------------////

#pragma once

#include "../ITest.hpp"

namespace Moonshot::Test
{
	class FunctionCalls : public ITest
	{
		public:
			FunctionCalls() = default;

			virtual std::string getTestName() const override;
			virtual bool runTest(Context & context) override;
		private:
			bool testFuncCall(Context& context, const std::string& str);
	};
}
