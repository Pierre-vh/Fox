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

TestManager::TestManager(Context& context) : context_(context)
{

}

TestManager::~TestManager()
{
}

void TestManager::addDefaultTests()
{
	// Lexer test
	std::unique_ptr<LexerMainTest> t(new LexerMainTest);
	addTest(std::move(t)); 
}

void TestManager::addTest(std::unique_ptr<ITest> test)
{
	tests_.emplace_back(std::move(test));
}


void TestManager::runTests(const bool& displayContextLog)
{
	std::cout << "-------------------------------------------------" << std::endl;
	context_.setMode(ContextLoggingMode::SAVE_TO_VECTOR);
	bool failflag = false;
	std::cout << "TestManager : Running tests..." << std::endl;
	std::cout << "=================================================" << std::endl;
	for (auto& elem : tests_)
	{
		std::cout << elem->getTestName() << "\t: ";
		if (elem->runTest(context_))
			std::cout << "SUCCESS";
		else
		{
			std::cout << "FAILURE";
			failflag = true;
		}
		std::cout << std::endl;
		if (displayContextLog)
		{
			std::cout << "Context log for this test:" << std::endl;
			context_.printLogs();
		}
	}
	// Display summary
	std::cout << "=================================================" << std::endl;
	std::cout << tests_.size() << " tests ran. Result: ";
	if (failflag)
		std::cout << "FAILURE";
	else
		std::cout << "SUCCESS";
	std::cout << std::endl <<	"-------------------------------------------------" << std::endl;
}
