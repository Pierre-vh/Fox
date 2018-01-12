////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : OptionsTests.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Unit tests for the options systems to check that it behaves correctly.
////------------------------------------------------------////

#pragma once

#include "../ITest.h"

#include "../../Common/Context/Options/OptionsManager.h"
#include "../../Common/Context/Options/ParameterValue.h"

namespace Moonshot
{
	class OptionsTests : public ITest
	{
		public:
			OptionsTests();
			~OptionsTests();

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


