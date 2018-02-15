////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ITest.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Base test class for each test.										
////------------------------------------------------------////

#pragma once

#include "Moonshot/Common/Context/Context.hpp" // context
#include "Moonshot/Common/Utils/Utils.hpp" // utils
#include "Moonshot/Common/Types/Types.hpp"

#include "Moonshot/Fox/Lexer/Lexer.hpp"
#include "Moonshot/Fox/Parser/Parser.hpp"

// STL
#include <fstream>
#include <vector>

#define FAILED_RETURN_IF_ERR(zone) 	if (!context.isSafe())	\
							{	\
								std::cout << "Test failed at " << zone << std::endl;	\
								return false;	\
							}
#define FAILED_RETURN_IF(cond,zone) if (cond)	\
									{	\
										std::cout << "Test failed at " << zone << std::endl;	\
										return false;	\
									}

#define FAILED_RETURN_IF_ERR__SILENT if(!context.isSafe()) return false


#define SUCCESS_CONTINUE_IF_ERR if (!context.isSafe()) \
								{ \
									std::cout << "\t\t\xC0 Success (Test Failed as Expected.)\n"; \
									continue; \
								}

#define SUCCESS_CONTINUE_IF(x)		if (x) \
									{ \
										std::cout << "\t\t\xC0 Success (Test Failed as Expected.)\n"; \
										continue; \
									}
namespace Moonshot::Test
{
	namespace TestUtilities
	{
		std::string readFileToString(Context& context, const std::string& fp);
		std::vector<std::string> readFileToVec(Context& context, const std::string& fp);

		static const std::string spacer_slim = "-------------------------------------------------";
		static const std::string spacer_large = "=================================================";
	}
	class ITest
	{
		public:
			ITest() = default;
			virtual ~ITest() = 0;

			// Returns the test's name
			virtual std::string getTestName() const = 0;
			// Runs the test, return false in case of failure, true if test is a success
			virtual bool runTest(Context& context) = 0;	// runs the test
	};
}
