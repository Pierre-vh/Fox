//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : DiagnosticsTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  Unit tests for the DiagnosticsEngine
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/Lexer/Lexer.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/Diagnostic.hpp"

using namespace fox;

namespace {
  class StrDiagConsumer : public DiagnosticConsumer {
    public:
      virtual void consume(Diagnostic& diag) override {
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
}

// Creates a DiagEngine
DiagnosticEngine createDiagEngine() {
  return DiagnosticEngine(std::make_unique<StrDiagConsumer>());
}

TEST(DiagnosticsTests, notes) {
  auto diagEng = createDiagEngine();
  auto diag = diagEng.report(DiagID::unittest_notetest);
  EXPECT_EQ("Test note", diag.getStr()) << "Diagnostic string did not match";
  EXPECT_EQ(DiagSeverity::NOTE, diag.getSeverity()) << "Diagnostic severity did not match";
  EXPECT_EQ(DiagID::unittest_notetest, diag.getID()) << "Diagnostic id did not match";
}

TEST(DiagnosticsTests, warnings) {
  auto diagEng = createDiagEngine();
  auto diag = diagEng.report(DiagID::unittest_warntest);
  EXPECT_EQ("Test warning", diag.getStr()) << "Diagnostic string did not match";
  EXPECT_EQ(DiagSeverity::WARNING, diag.getSeverity()) << "Diagnostic severity did not match";
  EXPECT_EQ(DiagID::unittest_warntest, diag.getID()) << "Diagnostic id did not match";
}

TEST(DiagnosticsTests, errors) {
  auto diagEng = createDiagEngine();
  auto diag = diagEng.report(DiagID::unittest_errtest);
  EXPECT_EQ("Test error", diag.getStr()) << "Diagnostic string did not match";
  EXPECT_EQ(DiagSeverity::ERROR, diag.getSeverity()) << "Diagnostic severity did not match";
  EXPECT_EQ(DiagID::unittest_errtest, diag.getID()) << "Diagnostic id did not match";
}

TEST(DiagnosticsTests, fatals) {
  auto diagEng = createDiagEngine();
  auto diag = diagEng.report(DiagID::unittest_fataltest);
  EXPECT_EQ("Test fatal", diag.getStr()) << "Diagnostic string did not match";
  EXPECT_EQ(DiagSeverity::FATAL, diag.getSeverity()) << "Diagnostic severity did not match";
  EXPECT_EQ(DiagID::unittest_fataltest, diag.getID()) << "Diagnostic id did not match";
}

TEST(DiagnosticsTests, emission) {
  auto diagEng = createDiagEngine();
  StrDiagConsumer* cons = static_cast<StrDiagConsumer*>(diagEng.getConsumer());
  EXPECT_EQ("", cons->getStr()) << "Consumer str wasn't empty at first.";
  // Test emission when diag goes out of scope
	{
    diagEng.report(DiagID::unittest_fataltest);
  }
  EXPECT_EQ("Test fatal", cons->getStr()) << "Consumer string did not match.";
}

//   NOTE(unittest_placeholderremoval1, "[%0,%1]")
TEST(DiagnosticsTests, addArg1) {
  auto diagEng = createDiagEngine();
  auto str = diagEng.report(DiagID::unittest_placeholderremoval1).addArg("foo").addArg(55.45f).getStr();
  EXPECT_EQ(str, "[foo,55.45]");
}

//   NOTE(unittest_placeholderremoval2, "[%0%0%0]")
TEST(DiagnosticsTests, addArg2) {
  auto diagEng = createDiagEngine();
  auto str = diagEng.report(DiagID::unittest_placeholderremoval2).addArg('a').getStr();
  EXPECT_EQ(str, "[aaa]");
}

//   NOTE(unittest_placeholderremoval3, "[%5%4%3%2%1%0]")
TEST(DiagnosticsTests, addArg3) {
  auto diagEng = createDiagEngine();
  auto str = diagEng.report(DiagID::unittest_placeholderremoval3).addArg('a').addArg('b').addArg('c').addArg('d').addArg('e').addArg('f').getStr();
  EXPECT_EQ(str, "[fedcba]");
}

//   NOTE(unittest_placeholderremoval4, "Hello, %0")
TEST(DiagnosticsTests, addArg4) {
  auto diagEng = createDiagEngine();
  auto str = diagEng.report(DiagID::unittest_placeholderremoval4).addArg("world").getStr();
  EXPECT_EQ(str, "Hello, world");
}

TEST(DiagnosticsTests, errLimit) {
  auto diagEng = createDiagEngine();
  StrDiagConsumer* cons = static_cast<StrDiagConsumer*>(diagEng.getConsumer());

  diagEng.setErrorLimit(1);
  EXPECT_FALSE(diagEng.hasFatalErrorOccured());

  auto diag1 = diagEng.report(DiagID::unittest_errtest);
  ASSERT_EQ(diag1.getSeverity(), DiagSeverity::ERROR);
  diag1.emit();
  // The last emitted error should have been a fatal error
  EXPECT_TRUE(diagEng.hasFatalErrorOccured());
  EXPECT_EQ(cons->getSev(), DiagSeverity::FATAL);
  EXPECT_EQ(cons->getID(), DiagID::diagengine_maxErrCountExceeded);

  auto count = cons->getCount();
  // Further diags should all be silenced.
  auto test_note = diagEng.report(DiagID::unittest_notetest);
  auto test_warn = diagEng.report(DiagID::unittest_warntest);
  auto test_err = diagEng.report(DiagID::unittest_errtest);
  auto test_fat = diagEng.report(DiagID::unittest_fataltest);

  EXPECT_EQ(test_note.getSeverity(), DiagSeverity::IGNORE);
  EXPECT_EQ(test_warn.getSeverity(), DiagSeverity::IGNORE);
  EXPECT_EQ(test_err.getSeverity(), DiagSeverity::IGNORE);
  EXPECT_EQ(test_fat.getSeverity(), DiagSeverity::IGNORE);

  test_note.emit();
  test_warn.emit();
  test_err.emit();
  test_fat.emit();

  EXPECT_EQ(count, cons->getCount());
}

TEST(DiagnosticsTests, frozenAndDeadDiags) {
  auto diagEng = createDiagEngine();
  auto diag = diagEng.report(DiagID::unittest_placeholderremoval1);
  EXPECT_EQ("[%0,%1]", diag.getStr()) << "Diag str wasn't the one expected.";
  diag.addArg("foo");
  EXPECT_EQ("[foo,%1]", diag.getStr()) << "Diag str did not replace the expected placeholder.";
  
  // Freeze test
  EXPECT_FALSE(diag.isFrozen()) << "Diag spawned frozen";
  diag.freeze();
  EXPECT_TRUE(diag.isFrozen()) << "Diag did not freeze as expected.";
  diag.addArg("bar");
  EXPECT_EQ("[foo,%1]", diag.getStr()) << "Diag str might have replaced a placeholder, but the diagnostic was supposed to be frozen!";
  
  // Alive/dead
  EXPECT_TRUE(diag.isActive()) << "Diag was inactive?";
  diag.emit();
  EXPECT_FALSE(diag.isActive()) << "Diag was active after being emitted?";
}

TEST(DiagnosticsTests, SilencedWarnings) {
  auto diagEng = createDiagEngine();
  // Set flag
  diagEng.setIgnoreWarnings(true);
  // Test.
  auto diagWarn = diagEng.report(DiagID::unittest_warntest);
  EXPECT_EQ(diagWarn.getSeverity(),DiagSeverity::IGNORE) << "Reported diagnostic wasn't a silenced diag";
  diagWarn.emit();
}

TEST(DiagnosticsTests, SilencedNotes) {
  auto diagEng = createDiagEngine();
  // Set flag
  diagEng.setIgnoreNotes(true);
  // Test.
  auto diagNote = diagEng.report(DiagID::unittest_notetest);
  EXPECT_EQ(diagNote.getSeverity(), DiagSeverity::IGNORE) << "Reported diagnostic wasn't a silenced diag";
  diagNote.emit();
}

TEST(DiagnosticsTests, SilenceAllAfterFatal) {
  auto diagEng = createDiagEngine();
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
  EXPECT_EQ(diagErrSilenced.getSeverity(), DiagSeverity::IGNORE) << "Diag was supposed to be silenced an thus this DiagID was supposed to be a Dummy diag.";
  diagErrSilenced.emit();
}

TEST(DiagnosticsTests, SilenceAll) {
  auto diagEng = createDiagEngine();
  diagEng.setIgnoreAll(true);
  auto dg1 = diagEng.report(DiagID::unittest_errtest);
  auto dg2 = diagEng.report(DiagID::unittest_warntest);
  auto dg3 = diagEng.report(DiagID::unittest_fataltest);

  EXPECT_EQ(dg1.getSeverity(), DiagSeverity::IGNORE) << "Diag was supposed to be silenced.";
  EXPECT_EQ(dg2.getSeverity(), DiagSeverity::IGNORE) << "Diag was supposed to be silenced.";
  EXPECT_EQ(dg3.getSeverity(), DiagSeverity::IGNORE) << "Diag was supposed to be silenced.";
}

TEST(DiagnosticsTests, WarningsAreErrors) {
  auto diagEng = createDiagEngine();
  diagEng.setWarningsAreErrors(true);
  diagEng.report(DiagID::unittest_warntest).emit();
  EXPECT_EQ(diagEng.getWarningsCount(), 0) << "Diag shouldn't have counted a normal warning";
  EXPECT_EQ(diagEng.getErrorsCount(), 1) << "Diag didn't count as an error.";
}

TEST(DiagnosticsTests, ErrorsAreFatal) {
  auto diagEng = createDiagEngine();
  diagEng.setErrorsAreFatal(true);
  diagEng.report(DiagID::unittest_errtest).emit();
  EXPECT_TRUE(diagEng.hasFatalErrorOccured()) << "Diag didn't count as a fatal error.";
  EXPECT_EQ(diagEng.getErrorsCount(), 0) << "This error was supposed to be fatal and thus count as a fatal error, not a normal error.";
}

TEST(DiagnosticsTests, CopyingDiagKillsCopiedDiag) {
  // Test with copy constructor
  auto diagEng = createDiagEngine();
  auto diagA = diagEng.report(DiagID::unittest_errtest);
  auto diagB(diagA);
  EXPECT_FALSE(diagA.isActive());
  EXPECT_TRUE(diagA.isFrozen());
  EXPECT_FALSE(diagA);

  EXPECT_TRUE(diagB.isActive());
  EXPECT_FALSE(diagB.isFrozen());
  EXPECT_TRUE(diagB);

  // Test with move constructor
  auto diagC(std::move(diagB));
  EXPECT_FALSE(diagB.isActive());
  EXPECT_TRUE(diagB.isFrozen());
  EXPECT_FALSE(diagB);

  EXPECT_TRUE(diagC.isActive());
  EXPECT_FALSE(diagC.isFrozen());
  EXPECT_TRUE(diagC);
}
