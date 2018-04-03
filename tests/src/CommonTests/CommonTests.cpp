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
	EXPECT_FALSE(fm.isSet(FlagID::unit_test_flag));
	fm.set(FlagID::unit_test_flag); // now it's set (true)
	EXPECT_TRUE(fm.isSet(FlagID::unit_test_flag));
	fm.unset(FlagID::unit_test_flag); // and unset again !
	EXPECT_FALSE(fm.isSet(FlagID::unit_test_flag));

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
	auto diag = diagEng.report(DiagID::unittest_errtest);
	EXPECT_EQ("Test error", diag.getDiagStr()) << "Diagnostic string did not match";
	EXPECT_EQ(DiagSeverity::ERROR, diag.getDiagSeverity()) << "Diagnostic severity did not match";
	EXPECT_EQ(DiagID::unittest_errtest, diag.getDiagID()) << "Diagnostic id did not match";
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
{
	auto diagEng = createDiagEngine();
	diagEng.setErrorLimit(1);
	EXPECT_FALSE(diagEng.hasFatalErrorOccured()) << "DiagnosticsEngine reported that a fatal error occured, but it was never used to report errors!";

	auto diag1 = diagEng.report(DiagID::unittest_errtest);
	EXPECT_EQ("Test error", diag1.getDiagStr());
	diag1.emit();
	EXPECT_FALSE(diagEng.hasFatalErrorOccured()) << "The DiagnosticsEngine reported a fatal error after 1 error.";

	auto diag2 = diagEng.report(DiagID::unittest_errtest);
	EXPECT_EQ(diag2.getDiagID(), DiagID::diagengine_maxErrCountExceeded) << "The report function did not return the expected diagnostic";
	EXPECT_EQ(diag2.getDiagSeverity(), DiagSeverity::FATAL) << "The report function did not return a fatal diagnostic";
	EXPECT_EQ("Current error count exceeded the maximum thresold of 1.", diag2.getDiagStr()) << "Incorrect diagstr";
	EXPECT_TRUE(diag2.isFrozen()) << "Diag was supposed to be frozen to prevent user modifications!";

	// Emit the diag and perform final check.
	diag2.emit();
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

TEST(DiagnosticsTests, FlagsAreCorrectlyApplied)
{
	auto diagEng = createDiagEngine();
	FlagsManager f1, f2;
	f1.set(FlagID::diagengine_errorsAreFatal);
	f2.set(FlagID::diagengine_silenceAll);
	f1.set(FlagID::diagengine_silenceAllAfterFatalError);
	f2.set(FlagID::diagengine_silenceNotes);
	f1.set(FlagID::diagengine_silenceWarnings);
	f2.set(FlagID::diagengine_warningsAreErrors);
	
	diagEng.setFlagsManager(&f1);
	EXPECT_TRUE(diagEng.getErrorsAreFatal());
	EXPECT_FALSE(diagEng.getSilenceAll());
	EXPECT_TRUE(diagEng.getSilenceAllAfterFatalErrors());
	EXPECT_FALSE(diagEng.getSilenceNotes());
	EXPECT_TRUE(diagEng.getSilenceWarnings());
	EXPECT_FALSE(diagEng.getWarningsAreErrors());

	diagEng.setFlagsManager(&f2);
	EXPECT_FALSE(diagEng.getErrorsAreFatal());
	EXPECT_TRUE(diagEng.getSilenceAll());
	EXPECT_FALSE(diagEng.getSilenceAllAfterFatalErrors());
	EXPECT_TRUE(diagEng.getSilenceNotes());
	EXPECT_FALSE(diagEng.getSilenceWarnings());
	EXPECT_TRUE(diagEng.getWarningsAreErrors());
}

TEST(DiagnosticsTests, SilencedWarnings)
{
	auto diagEng = createDiagEngine();
	// Set flag
	diagEng.setSilenceWarnings(true);
	// Test.
	auto diagWarn = diagEng.report(DiagID::unittest_warntest);
	EXPECT_EQ(diagWarn.getDiagID(), DiagID::dummyDiag) << "Reported diagnostic wasn't a silenced diag";
	diagWarn.emit();
}

TEST(DiagnosticsTests, SilencedNotes)
{
	auto diagEng = createDiagEngine();
	// Set flag
	diagEng.setSilenceNotes(true);
	// Test.
	auto diagNote = diagEng.report(DiagID::unittest_notetest);
	EXPECT_EQ(diagNote.getDiagID(), DiagID::dummyDiag) << "Reported diagnostic wasn't a silenced diag";
	diagNote.emit();
}

TEST(DiagnosticsTests, SilenceAllAfterFatal)
{
	auto diagEng = createDiagEngine();
	// Set flag
	diagEng.setSilenceAllAfterFatalErrors(true);
	// Test emission of an error
	diagEng.report(DiagID::unittest_errtest).emit();
	ASSERT_EQ(diagEng.getNumErrors(), 1) << "Error wasn't recorded?";
	
	// Report a fatal error
	diagEng.report(DiagID::unittest_fataltest).emit();
	EXPECT_EQ(diagEng.getNumErrors(), 1) << "Fatal error was counted like a normal error?";
	ASSERT_TRUE(diagEng.hasFatalErrorOccured()) << "Fatal error didn't count?";

	// And try to emit another error
	auto diagErrSilenced = diagEng.report(DiagID::unittest_errtest);
	EXPECT_EQ(diagErrSilenced.getDiagID(), DiagID::dummyDiag) << "Diag was supposed to be silenced an thus this DiagID was supposed to be a Dummy diag.";
	diagErrSilenced.emit();
}

TEST(DiagnosticsTests, SilenceAll)
{
	auto diagEng = createDiagEngine();
	diagEng.setSilenceAll(true);
	auto dg1 = diagEng.report(DiagID::unittest_errtest);
	auto dg2 = diagEng.report(DiagID::unittest_warntest);
	auto dg3 = diagEng.report(DiagID::unittest_fataltest);

	EXPECT_EQ(dg1.getDiagID(), DiagID::dummyDiag) << "Diag was supposed to be silenced.";
	EXPECT_EQ(dg2.getDiagID(), DiagID::dummyDiag) << "Diag was supposed to be silenced.";
	EXPECT_EQ(dg3.getDiagID(), DiagID::dummyDiag) << "Diag was supposed to be silenced.";
}

TEST(DiagnosticsTests, WarningsAreErrors)
{
	auto diagEng = createDiagEngine();
	diagEng.setWarningsAreErrors(true);
	diagEng.report(DiagID::unittest_warntest).emit();
	EXPECT_EQ(diagEng.getNumWarnings(), 0) << "Diag shouldn't have counted a normal warning";
	EXPECT_EQ(diagEng.getNumErrors(), 1) << "Diag didn't count as an error.";
}

TEST(DiagnosticsTests, ErrorsAreFatal)
{
	auto diagEng = createDiagEngine();
	diagEng.setErrorsAreFatal(true);
	diagEng.report(DiagID::unittest_errtest).emit();
	EXPECT_TRUE(diagEng.hasFatalErrorOccured()) << "Diag didn't count as a fatal error.";
	EXPECT_EQ(diagEng.getNumErrors(), 0) << "This error was supposed to be fatal and thus count as a fatal error, not a normal error.";
}