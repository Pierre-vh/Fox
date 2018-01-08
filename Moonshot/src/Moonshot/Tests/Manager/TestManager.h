////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TestManager.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Test manager class.										
////------------------------------------------------------////

#pragma once

#include "../ITest.h"
#include <memory>
#include <vector>


// Include default tests
#include "../Lexer/LexerMainTest.h"
#include "../ExprTest/ExprTests.h"

namespace Moonshot
{
	class TestManager
	{
		public:
			TestManager(Context& context);
			~TestManager();

			void addDefaultTests();
			void addTest(std::unique_ptr<ITest> test);
			void runTests(const bool& displayContextLog = false);

		private:
			template <typename T>
			inline void addTestClass()
			{
				std::unique_ptr<T> ptr(new T);
				addTest(std::move(ptr));
			}

			std::vector< std::unique_ptr<ITest> > tests_;
			Context& context_;
	};
}
