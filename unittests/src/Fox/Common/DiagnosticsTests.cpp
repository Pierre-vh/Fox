//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : DiagnosticsTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  Unit tests for the DiagnosticsEngine
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/Lexer/Lexer.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"

using namespace fox;

namespace {
  class StrDiagConsumer : public DiagnosticConsumer {
    public:
      virtual void consume(SourceManager& sm, const Diagnostic& diag) override {
        count_++;
        str_ = diag.getStr();
        sev_ = diag.getSeverity();
        id_ = diag.getID();
      }

      std::string getStr() const {
        return str_;
      }

      DiagSeverity getSev() const {
        return sev_;
      }

      DiagID getID() const {
        return id_;
      }
      
      std::uint8_t getCount() const {
        return count_;
      }

    private:
      std::uint8_t count_ = 0;
      DiagID id_;
      DiagSeverity sev_;
      std::string str_;
  };

  class DiagnosticsTest : public ::testing::Test {
    public:
      DiagnosticsTest() : diagEng(srcMgr) {}
    protected:
      SourceManager srcMgr;
      DiagnosticEngine diagEng;
  };
}
TEST_F(DiagnosticsTest, notes) {
  auto diag = diagEng.report(DiagID::unittest_notetest);
  EXPECT_EQ("Test note", diag.getStr()) << "Diagnostic string did not match";
  EXPECT_EQ(DiagSeverity::Note, diag.getSeverity()) << "Diagnostic severity did not match";
  EXPECT_EQ(DiagID::unittest_notetest, diag.getID()) << "Diagnostic id did not match";
}

TEST_F(DiagnosticsTest, warnings) {
  auto diag = diagEng.report(DiagID::unittest_warntest);
  EXPECT_EQ("Test warning", diag.getStr()) << "Diagnostic string did not match";
  EXPECT_EQ(DiagSeverity::Warning, diag.getSeverity()) << "Diagnostic severity did not match";
  EXPECT_EQ(DiagID::unittest_warntest, diag.getID()) << "Diagnostic id did not match";
}

TEST_F(DiagnosticsTest, errors) {
  auto diag = diagEng.report(DiagID::unittest_errtest);
  EXPECT_EQ("Test error", diag.getStr()) << "Diagnostic string did not match";
  EXPECT_EQ(DiagSeverity::Error, diag.getSeverity()) << "Diagnostic severity did not match";
  EXPECT_EQ(DiagID::unittest_errtest, diag.getID()) << "Diagnostic id did not match";
}

TEST_F(DiagnosticsTest, fatals) {
  auto diag = diagEng.report(DiagID::unittest_fataltest);
  EXPECT_EQ("Test fatal", diag.getStr()) << "Diagnostic string did not match";
  EXPECT_EQ(DiagSeverity::Fatal, diag.getSeverity()) << "Diagnostic severity did not match";
  EXPECT_EQ(DiagID::unittest_fataltest, diag.getID()) << "Diagnostic id did not match";
}

TEST_F(DiagnosticsTest, emission) {
  StrDiagConsumer* cons = static_cast<StrDiagConsumer*>(diagEng.getConsumer());
  EXPECT_EQ("", cons->getStr()) << "Consumer str wasn't empty at first.";
  // Test emission when diag goes out of scope
	{
    diagEng.report(DiagID::unittest_fataltest);
  }
  EXPECT_EQ("Test fatal", cons->getStr()) << "Consumer string did not match.";
}

//   NOTE(unittest_placeholderremoval1, "[%0,%1]")
TEST_F(DiagnosticsTest, addArg1) {
  auto str = diagEng.report(DiagID::unittest_placeholderremoval1).addArg("foo").addArg(55.45f).getStr();
  EXPECT_EQ(str, "[foo,55.45]");
}

//   NOTE(unittest_placeholderremoval2, "[%0%0%0]")
TEST_F(DiagnosticsTest, addArg2) {
  auto str = diagEng.report(DiagID::unittest_placeholderremoval2).addArg('a').getStr();
  EXPECT_EQ(str, "[aaa]");
}

//   NOTE(unittest_placeholderremoval3, "[%5%4%3%2%1%0]")
TEST_F(DiagnosticsTest, addArg3) {
  auto str = diagEng.report(DiagID::unittest_placeholderremoval3).addArg('a').addArg('b').addArg('c').addArg('d').addArg('e').addArg('f').getStr();
  EXPECT_EQ(str, "[fedcba]");
}

//   NOTE(unittest_placeholderremoval4, "Hello, %0")
TEST_F(DiagnosticsTest, addArg4) {
  auto str = diagEng.report(DiagID::unittest_placeholderremoval4).addArg("world").getStr();
  EXPECT_EQ(str, "Hello, world");
}

TEST_F(DiagnosticsTest, errLimit) {
  StrDiagConsumer* cons = static_cast<StrDiagConsumer*>(diagEng.getConsumer());

  diagEng.setErrorLimit(1);
  EXPECT_FALSE(diagEng.hasFatalErrorOccured());

  auto diag1 = diagEng.report(DiagID::unittest_errtest);
  ASSERT_EQ(diag1.getSeverity(), DiagSeverity::Error);
  diag1.emit();
  // The last emitted error should have been a fatal error
  EXPECT_TRUE(diagEng.hasFatalErrorOccured());
  EXPECT_EQ(cons->getSev(), DiagSeverity::Fatal);
  EXPECT_EQ(cons->getID(), DiagID::diagengine_maxErrCountExceeded);

  auto count = cons->getCount();
  // Further diags should all be silenced.
  auto test_note = diagEng.report(DiagID::unittest_notetest);
  auto test_warn = diagEng.report(DiagID::unittest_warntest);
  auto test_err = diagEng.report(DiagID::unittest_errtest);
  auto test_fat = diagEng.report(DiagID::unittest_fataltest);

  EXPECT_EQ(test_note.getSeverity(), DiagSeverity::Ignore);
  EXPECT_EQ(test_warn.getSeverity(), DiagSeverity::Ignore);
  EXPECT_EQ(test_err.getSeverity(), DiagSeverity::Ignore);
  EXPECT_EQ(test_fat.getSeverity(), DiagSeverity::Ignore);

  test_note.emit();
  test_warn.emit();
  test_err.emit();
  test_fat.emit();

  EXPECT_EQ(count, cons->getCount());
}

TEST_F(DiagnosticsTest, InactiveDiags) {
  auto diag = diagEng.report(DiagID::unittest_placeholderremoval1);
  EXPECT_EQ("[%0,%1]", diag.getStr()) 
    << "Diag str wasn't the one expected.";
  diag.addArg("foo");
  EXPECT_EQ("[foo,%1]", diag.getStr()) 
    << "Diag str did not replace the expected placeholder.";
  
  // Alive/dead
  EXPECT_TRUE(diag.isActive()) 
    << "Diag was inactive?";
  diag.emit();
  EXPECT_FALSE(diag.isActive()) 
    << "Diag was active after being emitted?";
}

TEST_F(DiagnosticsTest, SilencedWarnings) {
  // Set flag
  diagEng.setIgnoreWarnings(true);
  // Test.
  auto diagWarn = diagEng.report(DiagID::unittest_warntest);
  EXPECT_EQ(diagWarn.getSeverity(),DiagSeverity::Ignore) 
    << "Reported diagnostic wasn't a silenced diag";
  diagWarn.emit();
}

TEST_F(DiagnosticsTest, SilencedNotes) {
  // Set flag
  diagEng.setIgnoreNotes(true);
  // Test.
  auto diagNote = diagEng.report(DiagID::unittest_notetest);
  EXPECT_EQ(diagNote.getSeverity(), DiagSeverity::Ignore) << "Reported diagnostic wasn't a silenced diag";
  diagNote.emit();
}

TEST_F(DiagnosticsTest, SilenceAllAfterFatal) {
  // Set flag
  diagEng.setIgnoreAllAfterFatal(true);
  // Test emission of an error
  diagEng.report(DiagID::unittest_errtest).emit();
  ASSERT_EQ(diagEng.getErrorsCount(), 1) << "Error wasn't recorded?";
  
  // Report a fatal error
  diagEng.report(DiagID::unittest_fataltest).emit();
  EXPECT_EQ(diagEng.getErrorsCount(), 1) << "Fatal error was counted like a normal error?";
  ASSERT_TRUE(diagEng.hasFatalErrorOccured()) << "Fatal error didn't count?";

  // And try to emit another error
  auto diagErrSilenced = diagEng.report(DiagID::unittest_errtest);
  EXPECT_EQ(diagErrSilenced.getSeverity(), DiagSeverity::Ignore) << "Diag was supposed to be silenced an thus this DiagID was supposed to be a Dummy diag.";
  diagErrSilenced.emit();
}

TEST_F(DiagnosticsTest, SilenceAll) {
  diagEng.setIgnoreAll(true);
  auto dg1 = diagEng.report(DiagID::unittest_errtest);
  auto dg2 = diagEng.report(DiagID::unittest_warntest);
  auto dg3 = diagEng.report(DiagID::unittest_fataltest);

  EXPECT_EQ(dg1.getSeverity(), DiagSeverity::Ignore) << "Diag was supposed to be silenced.";
  EXPECT_EQ(dg2.getSeverity(), DiagSeverity::Ignore) << "Diag was supposed to be silenced.";
  EXPECT_EQ(dg3.getSeverity(), DiagSeverity::Ignore) << "Diag was supposed to be silenced.";
}

TEST_F(DiagnosticsTest, WarningsAreErrors) {
  diagEng.setWarningsAreErrors(true);
  diagEng.report(DiagID::unittest_warntest).emit();
  EXPECT_EQ(diagEng.getWarningsCount(), 0) << "Diag shouldn't have counted a normal warning";
  EXPECT_EQ(diagEng.getErrorsCount(), 1) << "Diag didn't count as an error.";
}

TEST_F(DiagnosticsTest, ErrorsAreFatal) {
  diagEng.setErrorsAreFatal(true);
  diagEng.report(DiagID::unittest_errtest).emit();
  EXPECT_TRUE(diagEng.hasFatalErrorOccured()) << "Diag didn't count as a fatal error.";
  EXPECT_EQ(diagEng.getErrorsCount(), 0) << "This error was supposed to be fatal and thus count as a fatal error, not a normal error.";
}

TEST_F(DiagnosticsTest, CopyingDiagKillsCopiedDiag) {
  // Test with copy constructor
  auto diagA = diagEng.report(DiagID::unittest_errtest);
  auto diagB(diagA);
  EXPECT_FALSE(diagA.isActive());
  EXPECT_FALSE(diagA);

  EXPECT_TRUE(diagB.isActive());
  EXPECT_TRUE(diagB);

  // Test with move constructor
  auto diagC(std::move(diagB));
  EXPECT_FALSE(diagB.isActive());
  EXPECT_FALSE(diagB);

  EXPECT_TRUE(diagC.isActive());
  EXPECT_TRUE(diagC);
}
