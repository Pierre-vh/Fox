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

#include "TestDiagCons.hpp"
#include "Moonshot/Common/Diagnostics/DiagnosticEngine.hpp"
#include "Moonshot/Common/Diagnostics/Diagnostic.hpp"

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
TEST(FlagManagerTest,FlagFunctions)
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

// Diagnostic system
TEST(Diagnostics, notes)
{
	DiagnosticEngine deg;
	auto diag = deg.report(DiagsID::unittest_notetest);
	EXPECT_EQ("Test note", diag.getDiagStr()) << "Diagnostic string did not match";
	EXPECT_EQ(DiagSeverity::NOTE, diag.getDiagSeverity()) << "Diagnostic severity did not match";
	EXPECT_EQ(DiagsID::unittest_notetest, diag.getDiagID()) << "Diagnostic id did not match";
}

TEST(Diagnostics, warnings)
{
	DiagnosticEngine deg;
	auto diag = deg.report(DiagsID::unittest_warntest);
	EXPECT_EQ("Test warning", diag.getDiagStr()) << "Diagnostic string did not match";
	EXPECT_EQ(DiagSeverity::WARNING, diag.getDiagSeverity()) << "Diagnostic severity did not match";
	EXPECT_EQ(DiagsID::unittest_warntest, diag.getDiagID()) << "Diagnostic id did not match";
}

TEST(Diagnostics, errors)
{
	DiagnosticEngine deg;
	auto diag = deg.report(DiagsID::unittests_errtest);
	EXPECT_EQ("Test error", diag.getDiagStr()) << "Diagnostic string did not match";
	EXPECT_EQ(DiagSeverity::ERROR, diag.getDiagSeverity()) << "Diagnostic severity did not match";
	EXPECT_EQ(DiagsID::unittests_errtest, diag.getDiagID()) << "Diagnostic id did not match";
}

TEST(Diagnostics, fatals)
{
	DiagnosticEngine deg;
	auto diag = deg.report(DiagsID::unittest_fataltest);
	EXPECT_EQ("Test fatal", diag.getDiagStr()) << "Diagnostic string did not match";
	EXPECT_EQ(DiagSeverity::FATAL, diag.getDiagSeverity()) << "Diagnostic severity did not match";
	EXPECT_EQ(DiagsID::unittest_fataltest, diag.getDiagID()) << "Diagnostic id did not match";
}

TEST(Diagnostics, emission)
{
	DiagnosticEngine deg(std::make_unique<StrDiagConsumer>());
	StrDiagConsumer* cons = dynamic_cast<StrDiagConsumer*>(deg.getConsumer());
	EXPECT_EQ("", cons->getStr()) << "Consumer str wasn't empty at first.";
	// Test emission when diag goes out of scope
	{
		deg.report(DiagsID::unittest_fataltest);
	}
	EXPECT_EQ("Test fatal", cons->getStr()) << "Consumer string did not match.";
}