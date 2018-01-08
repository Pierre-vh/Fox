////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ITest.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Base test class for each test.										
////------------------------------------------------------////

#pragma once

#include "../../src/Moonshot/Common/Context/Context.h" // context
#include "../../src/Moonshot/Common/Utils/Utils.h" // utils


#include "../../src/Moonshot/Fox/Lexer/Lexer.h"
#include "../../src/Moonshot/Fox/Parser/Parser.h"

// STL
#include <fstream>
#include <vector>

namespace Moonshot
{
	namespace TestUtilities
	{
		std::string readFileToString(Context& context, const std::string& fp);
		std::vector<std::string> readFileToVec(Context& context, const std::string& fp);
	}
	class ITest
	{
		public:
			ITest() = default;
			virtual ~ITest() = 0;

			virtual std::string getTestName() const = 0;
			virtual bool runTest(Context& context) = 0;	// runs the test
	};
}
