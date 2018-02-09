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


// Include default tests
#include "../Lexer/LexerMainTest.hpp"
#include "../Expressions/ExprTests.hpp"
#include "../ExprStmt/ExprStmt.hpp"
#include "../Options/OptionsTests.hpp"
#include "../VarDeclarations/VarDeclarations.hpp"
#include "../VarStmts/VarStmts.hpp"
#include "../CompoundStatement/CompoundStatements.hpp"
#include "../Condition/Conditions.hpp"
#include "../WhileLoop/WhileLoop.hpp"
#include "../U8StrManipTest/U8StrManipTest.hpp"

namespace Moonshot
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
