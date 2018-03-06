////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TestManager.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Test manager class.										
////------------------------------------------------------////

#pragma once

#include "../ITest.hpp"
#include <memory>
#include <vector>

namespace Moonshot::Test
{
	class TestManager
	{
		public:
			TestManager(Context& context);
			~TestManager();

			// add a default test
			void addDefaultTests();
			// Add a specific test
			void addTest(std::unique_ptr<ITest> test);
			// Run all tests
			void runTests(const bool& displayContextLog = false);

		private:
			template <typename T>
			inline void addTestClass()
			{
				auto ptr = std::make_unique<T>();
				addTest(std::move(ptr));
			}

			std::vector< std::unique_ptr<ITest> > tests_;
			Context& context_;
	};
}
