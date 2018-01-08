////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ITest.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Base test class for each test.										
////------------------------------------------------------////

#pragma once

#include "../../Moonshot/Common/Context/Context.h" // context
#include "../../Moonshot/Common/Utils/Utils.h" // utils
#include "../../Moonshot/Common/Types/Types.h"


#include "../../Moonshot/Fox/Lexer/Lexer.h"
#include "../../Moonshot/Fox/Parser/Parser.h"

// STL
#include <fstream>
#include <vector>

#define RETURN_IF_ERR(zone) 	if (!context.isSafe())	\
							{	\
								std::cout << "Test failed at " << zone << std::endl;	\
								return false;	\
							}

#define RETURN_SILENTLY_IF_ERR if(!context.isSafe()) return false;

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
