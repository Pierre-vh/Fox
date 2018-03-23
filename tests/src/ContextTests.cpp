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

TEST(ContextTests, ClearLogs)
{
	Context ctxt(Context::LoggingMode::SAVE_TO_VECTOR);
	ctxt.reportError("Error!");
	ctxt.reportError("Another!");
	ctxt.reportWarning("Attention!");
	EXPECT_TRUE(ctxt.getLogs().size()) << "Context::getLogs()::size() was 0 (no logs were saved)";
	ctxt.clearLogs();
	EXPECT_FALSE(ctxt.getLogs().size()) << "Context::getLogs()::size() wasn't null (logs were kept even though clearLogs was called)";
}

// Write tests for ContextLoggingModes