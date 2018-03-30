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

// Creates a DiagEngine
DiagnosticEngine createDiagEngine()
{
	return DiagnosticEngine(std::make_unique<StrDiagConsumer>());
}

TEST(DiagnosticsTests, notes)
{
	auto diagEng = createDiagEngine();
	auto diag = diagEng.report(DiagID::unittest_notetest);
	EXPECT_EQ("Test note", diag.getDiagStr()) << "Diagnostic string did not match";
	EXPECT_EQ(DiagSeverity::NOTE, diag.getDiagSeverity()) << "Diagnostic severity did not match";
	EXPECT_EQ(DiagID::unittest_notetest, diag.getDiagID()) << "Diagnostic id did not match";
}

TEST(DiagnosticsTests, warnings)
{
	auto diagEng = createDiagEngine();
	auto diag = diagEng.report(DiagID::unittest_warntest);
	EXPECT_EQ("Test warning", diag.getDiagStr()) << "Diagnostic string did not match";
	EXPECT_EQ(DiagSeverity::WARNING, diag.getDiagSeverity()) << "Diagnostic severity did not match";
	EXPECT_EQ(DiagID::unittest_warntest, diag.getDiagID()) << "Diagnostic id did not match";
}

TEST(DiagnosticsTests, errors)
{
	auto diagEng = createDiagEngine();
	auto diag = diagEng.report(DiagID::unittests_errtest);
	EXPECT_EQ("Test error", diag.getDiagStr()) << "Diagnostic string did not match";
	EXPECT_EQ(DiagSeverity::ERROR, diag.getDiagSeverity()) << "Diagnostic severity did not match";
	EXPECT_EQ(DiagID::unittests_errtest, diag.getDiagID()) << "Diagnostic id did not match";
}

TEST(DiagnosticsTests, fatals)
{
	auto diagEng = createDiagEngine();
	auto diag = diagEng.report(DiagID::unittest_fataltest);
	EXPECT_EQ("Test fatal", diag.getDiagStr()) << "Diagnostic string did not match";
	EXPECT_EQ(DiagSeverity::FATAL, diag.getDiagSeverity()) << "Diagnostic severity did not match";
	EXPECT_EQ(DiagID::unittest_fataltest, diag.getDiagID()) << "Diagnostic id did not match";
}

TEST(DiagnosticsTests, emission)
{
	auto diagEng = createDiagEngine();
	StrDiagConsumer* cons = dynamic_cast<StrDiagConsumer*>(diagEng.getConsumer());
	EXPECT_EQ("", cons->getStr()) << "Consumer str wasn't empty at first.";
	// Test emission when diag goes out of scope
	{
		diagEng.report(DiagID::unittest_fataltest);
	}
	EXPECT_EQ("Test fatal", cons->getStr()) << "Consumer string did not match.";
}

// 	NOTE(unittest_placeholderremoval1, "[%0,%1]")
TEST(DiagnosticsTests, addArg1)
{
	auto diagEng = createDiagEngine();
	auto str = diagEng.report(DiagID::unittest_placeholderremoval1).addArg("foo").addArg(55.45f).getDiagStr();
	EXPECT_EQ(str, "[foo,55.45]");
}

// 	NOTE(unittest_placeholderremoval2, "[%0%0%0]")
TEST(DiagnosticsTests, addArg2)
{
	auto diagEng = createDiagEngine();
	auto str = diagEng.report(DiagID::unittest_placeholderremoval2).addArg('a').getDiagStr();
	EXPECT_EQ(str, "[aaa]");
}

// 	NOTE(unittest_placeholderremoval3, "[%5%4%3%2%1%0]")
TEST(DiagnosticsTests, addArg3)
{
	auto diagEng = createDiagEngine();
	auto str = diagEng.report(DiagID::unittest_placeholderremoval3).addArg('a').addArg('b').addArg('c').addArg('d').addArg('e').addArg('f').getDiagStr();
	EXPECT_EQ(str, "[fedcba]");
}

// 	NOTE(unittest_placeholderremoval4, "Hello, %0")
TEST(DiagnosticsTests, addArg4)
{
	auto diagEng = createDiagEngine();
	auto str = diagEng.report(DiagID::unittest_placeholderremoval4).addArg("world").getDiagStr();
	EXPECT_EQ(str, "Hello, world");
}

TEST(DiagnosticsTests, errLimitWorks)
{
	auto diagEng = createDiagEngine();
	diagEng.setErrorLimit(1);
	EXPECT_FALSE(diagEng.hasFatalErrorOccured()) << "DiagnosticsEngine reported that a fatal error occured, but it was never used to report errors!";

	diagEng.report(DiagID::unittests_errtest).emit();
	EXPECT_FALSE(diagEng.hasFatalErrorOccured()) << "The DiagnosticsEngine reported a fatal error after 1 error.";

	diagEng.report(DiagID::unittests_errtest).emit();

	EXPECT_TRUE(diagEng.hasFatalErrorOccured()) << "Fatal error did not occur. Current error count:" << diagEng.getNumErrors() << " Error limit:" << diagEng.getErrorLimit();
}