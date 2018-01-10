////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TestManager.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "TestManager.h"

using namespace Moonshot;
using namespace Moonshot::TestUtilities;

TestManager::TestManager(Context& context) : context_(context)
{

}

TestManager::~TestManager()
{
}

void TestManager::addDefaultTests()
{
	addTestClass<LexerMainTest>(); // Lexer Test
	addTestClass<ExprTests>(); // Expression test
	addTestClass<OptionsTests>();
}

void TestManager::addTest(std::unique_ptr<ITest> test)
{
	tests_.emplace_back(std::move(test));
}


void TestManager::runTests(const bool& displayContextLog)
{
	std::cout << spacer_slim << std::endl;
	context_.setLoggingMode(ContextLoggingMode::SAVE_TO_VECTOR);
	bool failflag = false;
	std::cout << "TestManager : Running tests..." << std::endl;
	std::cout << spacer_large << std::endl;
	for (auto& elem : tests_)
	{
		std::cout << "[" << elem->getTestName() << "]" << std::endl;
		if (elem->runTest(context_))
			std::cout << "\tTest SUCCESSFUL" << std::endl;
		else
		{
			std::cout << "\tTest FAILED" << std::endl;
			failflag = true;
		}
		if (displayContextLog && failflag)
		{
			std::cout << std::endl << "Context log for this test:" << std::endl;
			context_.printLogs();
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
