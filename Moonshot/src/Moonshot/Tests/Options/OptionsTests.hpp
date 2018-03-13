////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : OptionsTests.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Unit tests for the options systems to check that it behaves correctly.
////------------------------------------------------------////

#pragma once

#include "../ITest.hpp"

#include "Moonshot/Common/Context/Options/OptionsManager.hpp"
#include "Moonshot/Common/Context/Options/ParameterValue.hpp"

namespace Moonshot::Test
{
	class OptionsTests : public ITest
	{
		public:
			OptionsTests() = default;

			virtual std::string getTestName() const override;
			virtual bool runTest(Context & context) override;

		private:
			// Specific tests:
			// OptionsManager
			bool testOptManagerFunc(Context & context, OptionsManager & options);
			// ParameterValue
			bool testParamvalueFuncs(Context & context);
	};
}


