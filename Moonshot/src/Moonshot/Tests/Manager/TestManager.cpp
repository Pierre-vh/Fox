////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TestManager.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "TestManager.hpp"

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
#include "../FuncDecl/FuncDecl.hpp"

using namespace Moonshot;
using namespace Moonshot::Test;
using namespace Moonshot::Test::TestUtilities;

TestManager::TestManager(Context& context) : context_(context)
{

}

TestManager::~TestManager()
{
}

void TestManager::addDefaultTests()
{
	addTestClass<OptionsTests>();
	addTestClass<LexerMainTest>(); // Lexer Test
	addTestClass<ExprTests>(); // Expression test
	addTestClass<ExprStmtTest>();
	addTestClass<VarDeclarations>();
	addTestClass<VarStmts>();
	addTestClass<CompoundStatements>();
	addTestClass<Conditions>();
	addTestClass<WhileLoop>();
	addTestClass<U8StrManipTest>();
	addTestClass<FuncDecl>();
}

void TestManager::addTest(std::unique_ptr<ITest> test)
{
	tests_.emplace_back(std::move(test));
}


void TestManager::runTests(const bool& displayContextLog)
{
	std::cout << spacer_slim << std::endl;
	context_.setLoggingMode(Context::LoggingMode::SAVE_TO_VECTOR);
	bool failflag = false;
	std::cout << "TestManager : Running tests...\n";
	std::cout << spacer_large << std::endl;
	for (const auto& elem : tests_)
	{
		std::cout << "[" << elem->getTestName() << "]\n";
		if (elem->runTest(context_))
			std::cout << "\tTest SUCCESSFUL\n";
		else
		{
			std::cout << "\tTest FAILED\n";
			failflag = true;
		}
		if (displayContextLog && failflag)
		{
			std::cout << std::endl << "Context log for this test:\n";
			context_.printLogs();
			break;
		}
		context_.resetState();
		context_.clearLogs();
		std::cout << spacer_slim << std::endl;
	}
	// Display summary
	std::cout << spacer_large << std::endl;
	std::cout << tests_.size() << " tests ran. Result: ";
	if (failflag)
		std::cout << "FAILURE";
	else
		std::cout << "SUCCESS";
	std::cout << std::endl << spacer_slim << std::endl;
}
