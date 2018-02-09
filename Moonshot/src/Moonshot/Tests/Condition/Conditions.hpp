////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Conditions.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Test class for conditions (if/else if/else)
// uses test file located in test/conditions/bad and /correct
////------------------------------------------------------////

#pragma once

#include "../ITest.hpp"

namespace Moonshot
{
	class Conditions : public ITest
	{
		public:
			Conditions() = default;
			~Conditions();

			virtual std::string getTestName() const override;
			virtual bool runTest(Context & context) override;
		private:
			bool testCond(Context & context,const std::string& str);
	};
}
