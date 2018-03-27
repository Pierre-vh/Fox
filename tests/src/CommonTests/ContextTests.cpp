////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ContextTests.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//	(Unit) Tests for the Context module.
////------------------------------------------------------////

#include "gtest/gtest.h"
#include "Moonshot/Fox/Lexer/Lexer.hpp"
#include "Moonshot/Common/Context/Context.hpp"

using namespace Moonshot;

TEST(ContextTests, ErrorReporting)
{
	Context ctxt(Context::LoggingMode::SILENT);
	ctxt.reportError("Error!");
	EXPECT_FALSE(ctxt.isSafe()) << "Context was safe even though errors were reported";
	ctxt.reportFatalError("Fatal error!");
	EXPECT_TRUE(ctxt.isCritical()) << "Context was not critical even tough a fatal error was reported.";
}

TEST(ContextTests, Reset)
{
	Context ctxt(Context::LoggingMode::SILENT);
	ctxt.reportError("Error!");
	EXPECT_FALSE(ctxt.isSafe()) << "Context was safe even tough errors were reported";
	ctxt.resetState();
	EXPECT_TRUE(ctxt.isSafe()) << "Context did not go back to a normal state even though reset was called.";
}

TEST(ContextTests, SaveToVecMode)
{
	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	ctxt.reportError("Error!");
	ctxt.reportError("Another!");
	ctxt.reportWarning("Attention!");
	EXPECT_TRUE(ctxt.getLogs().size()) << "Context::getLogs()::size() was 0 (no logs were saved)";
	ctxt.clearLogs();
	EXPECT_FALSE(ctxt.getLogs().size()) << "Context::getLogs()::size() wasn't 0 (logs were kept even though clearLogs was called)";
}

TEST(ContextTests, SilentMode)
{
	Context ctxt(Context::LoggingMode::SILENT);
	ctxt.reportError("Error!");
	EXPECT_FALSE(ctxt.getLogs().size()) << "Context::getLogs()::size() was not 0 (logs were saved even though silent mode was active)";
}

// Flagtests
TEST(ContextTests, FlagManager)
{
	// This test tests the 3 major functions of the flagmanager : isSet, set and unset
	Context ctxt;
	auto fm = ctxt.flagsManager();
	// The base value of the test flag is false, so it's expected to be false.
	EXPECT_FALSE(fm.isSet(CommonFlag::unit_test_flag));
	fm.set(CommonFlag::unit_test_flag); // now it's set (true)
	EXPECT_TRUE(fm.isSet(CommonFlag::unit_test_flag));
	fm.unset(CommonFlag::unit_test_flag); // and unset again !
	EXPECT_FALSE(fm.isSet(CommonFlag::unit_test_flag));

}