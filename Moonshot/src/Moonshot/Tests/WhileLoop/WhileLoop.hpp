////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : WhileLoop.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Tests for While loop. Uses the test files located under /res/tests/whileloop										
////------------------------------------------------------////

#pragma once

#include "../ITest.hpp"

namespace Moonshot::Test
{
	class WhileLoop : public ITest
	{
		public:
			WhileLoop() = default;

			virtual std::string getTestName() const override;
			virtual bool runTest(Context & context) override;
		private:
			bool testWhileLoop(Context& context, const std::string& str);
	};
}
