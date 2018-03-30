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

TEST(DiagnosticsTests, errLimit)
*{
	auto diagEng = createDiagEngine();
	diagEng.setErrorLimit(1);
	EXPECT_FALSE(diagEng.hasFatalErrorOccured()) << "DiagnosticsEngine reported that a fatal error occured, but it was never used to report errors!";

	diagEng.report(DiagID::unittests_errtest).emit();
	EXPECT_FALSE(diagEng.hasFatalErrorOccured()) << "The DiagnosticsEngine reported a fatal error after 1 error.";

	auto diag = diagEng.report(DiagID::unittests_errtest);
	EXPECT_EQ(diag.getDiagID(), DiagID::diagengine_maxErrCountExceeded) << "The report function did not return the expected diagnostic";
	EXPECT_EQ(diag.getDiagSeverity(), DiagSeverity::FATAL) << "The report function did not return a fatal diagnostic";
	EXPECT_EQ("Current error count exceeded the maximum thresold of 1.", diag.getDiagStr()) << "Incorrect diagstr";
	EXPECT_TRUE(diag.isFrozen()) << "Diag was supposed to be frozen to prevent user modifications!";

	// Emit the diag and perform final check.
	diag.emit();
	EXPECT_TRUE(diagEng.hasFatalErrorOccured()) << "Fatal error did not occur. Current error count: " << diagEng.getNumErrors() << "; Error limit: " << diagEng.getErrorLimit();
}

TEST(DiagnosticsTests, frozenAndDeadDiags)
{
	auto diagEng = createDiagEngine();
	auto diag = diagEng.report(DiagID::unittest_placeholderremoval1);
	EXPECT_EQ("[%0,%1]", diag.getDiagStr()) << "Diag str wasn't the one expected.";
	diag.addArg("foo", 1);
	EXPECT_EQ("[%0,foo]", diag.getDiagStr()) << "Diag str did not replace the expected placeholder.";
	
	// Freeze test
	EXPECT_FALSE(diag.isFrozen()) << "Diag spawned frozen";
	diag.freeze();
	EXPECT_TRUE(diag.isFrozen()) << "Diag did not freeze as expected.";
	diag.addArg("bar");
	EXPECT_EQ("[%0,foo]", diag.getDiagStr()) << "Diag str might have replaced a placeholder, but the diagnostic was supposed to be frozen!";
	
	// Alive/dead
	EXPECT_TRUE(diag.isActive()) << "Diag was inactive?";
	diag.emit();
	EXPECT_FALSE(diag.isActive()) << "Diag was active after being emitted?";
}

TEST(DiagnosticsTests, dummyDiags)
{
	auto diagEng = createDiagEngine();
	auto diag = Diagnostic::createDummyDiagnosticObject();
	// TESTS !
	EXPECT_FALSE(diag.isActive())							<< "Diag was active, but was expected to spawn inactive.";
	EXPECT_TRUE(diag.isFrozen())							<< "Diag wasn't frozen, but was expected to spawn frozen.";
	EXPECT_EQ(diag.getDiagID(), DiagID::dummyDiag)			<< "DiagID was not the one expected!";
	EXPECT_FALSE(diag.hasValidConsumer())					<< "Diag was supposed to not have any consumer set.";
	EXPECT_EQ("", diag.getDiagStr())						<< "DiagStr was supposed to be a empty string \"\"";
	EXPECT_EQ(DiagSeverity::IGNORE, diag.getDiagSeverity()) << "Diag's severity was supposed to be \"IGNORE\"";
}

/* 
Tests to write 
Note: most of theses are implicitely proven to be working by the tests above, but i
prefer to write more tests, so in the future if the projects gets more complicated I can
find where the issue is much more quickly!
	DiagEngine:
		- Check if flagmanager options are correctly applied by creating flagsmanager
		with different configurations & creating DiagEngines with them and check the getters.
		btw, check if resetAll works.
		
		- Tests for:
			warningsAreErrors
			errorsAreFatal
			silenceWarnings
			silenceNotes
			silenceAllAfterFatalErrors
			silenceAll
			getNumWarnings & getNumErrors

	Then finally move on to SourceManager & SourceLoc. This will be a bit long to do, but not much longer
	than the DiagEngine I hope.
*/