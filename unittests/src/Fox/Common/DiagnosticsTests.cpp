//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : DiagnosticsTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  Unit tests for the DiagnosticsEngine
//
//  Several types of tests are contained in this file:
//
//    DiagnosticsTest -> General Diagnostic/DiagnosticEngine tests 
//                      (e.g. formatting, emission)
//
//    PrettyDiagConsumerTest -> Tests for the "StreamDiagConsumer", which is
//                              the default DiagnosticEngine that pretty prints
//                              diagnostics to a stream.
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/Lexer/Lexer.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/SourceManager.hpp"

using namespace fox;

namespace {
  class StrDiagConsumer : public DiagnosticConsumer {
    public:
      virtual void consume(SourceManager&, const Diagnostic& diag) override {
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
      DiagnosticsTest() : diagEng(srcMgr, std::make_unique<StrDiagConsumer>()) {
        cons = static_cast<StrDiagConsumer*>(diagEng.getConsumer());
        file = srcMgr.loadFromString("", "foo");
      }
    protected:
      FileID file;
      StrDiagConsumer* cons = nullptr;
      SourceManager srcMgr;
      DiagnosticEngine diagEng;
  };

  class PrettyDiagConsumerTest : public ::testing::Test {
    public:
      PrettyDiagConsumerTest() : diagEng(srcMgr, ss) {}

    protected:
      std::stringstream ss;
      SourceManager srcMgr;
      DiagnosticEngine diagEng;
  };
}
TEST_F(DiagnosticsTest, notes) {
  auto diag = diagEng.report(DiagID::unittest_notetest, file);
  EXPECT_EQ("Test note", diag.getStr()) << "Diagnostic string did not match";
  EXPECT_EQ(DiagSeverity::Note, diag.getSeverity()) << "Diagnostic severity did not match";
  EXPECT_EQ(DiagID::unittest_notetest, diag.getID()) << "Diagnostic id did not match";
}

TEST_F(DiagnosticsTest, warnings) {
  auto diag = diagEng.report(DiagID::unittest_warntest, file);
  EXPECT_EQ("Test warning", diag.getStr()) << "Diagnostic string did not match";
  EXPECT_EQ(DiagSeverity::Warning, diag.getSeverity()) << "Diagnostic severity did not match";
  EXPECT_EQ(DiagID::unittest_warntest, diag.getID()) << "Diagnostic id did not match";
}

TEST_F(DiagnosticsTest, errors) {
  auto diag = diagEng.report(DiagID::unittest_errtest, file);
  EXPECT_EQ("Test error", diag.getStr()) << "Diagnostic string did not match";
  EXPECT_EQ(DiagSeverity::Error, diag.getSeverity()) << "Diagnostic severity did not match";
  EXPECT_EQ(DiagID::unittest_errtest, diag.getID()) << "Diagnostic id did not match";
}

TEST_F(DiagnosticsTest, fatals) {
  auto diag = diagEng.report(DiagID::unittest_fataltest, file);
  EXPECT_EQ("Test fatal", diag.getStr()) << "Diagnostic string did not match";
  EXPECT_EQ(DiagSeverity::Fatal, diag.getSeverity()) << "Diagnostic severity did not match";
  EXPECT_EQ(DiagID::unittest_fataltest, diag.getID()) << "Diagnostic id did not match";
}

TEST_F(DiagnosticsTest, emission) {
  EXPECT_EQ("", cons->getStr()) << "Consumer str wasn't empty at first.";
  // Test emission when diag goes out of scope
	{
    diagEng.report(DiagID::unittest_fataltest, file);
  }
  EXPECT_EQ("Test fatal", cons->getStr()) << "Consumer string did not match.";
}

//   NOTE(unittest_placeholderremoval1, "[%0,%1]")
TEST_F(DiagnosticsTest, addArg1) {
  auto str = diagEng.report(DiagID::unittest_placeholderremoval1, file)
    .addArg("foo").addArg(55.45f).getStr();
  EXPECT_EQ(str, "[foo,55.45]");
}

//   NOTE(unittest_placeholderremoval2, "[%0%0%0]")
TEST_F(DiagnosticsTest, addArg2) {
  auto str = diagEng.report(DiagID::unittest_placeholderremoval2, file)
    .addArg('a').getStr();
  EXPECT_EQ(str, "[aaa]");
}

//   NOTE(unittest_placeholderremoval3, "[%5%4%3%2%1%0]")
TEST_F(DiagnosticsTest, addArg3) {
  auto str = diagEng.report(DiagID::unittest_placeholderremoval3, file)
    .addArg('a').addArg('b').addArg('c')
    .addArg('d').addArg('e').addArg('f').getStr();
  EXPECT_EQ(str, "[fedcba]");
}

//   NOTE(unittest_placeholderremoval4, "Hello, %0")
TEST_F(DiagnosticsTest, addArg4) {
  auto str = diagEng.report(DiagID::unittest_placeholderremoval4, file)
    .addArg("world").getStr();
  EXPECT_EQ(str, "Hello, world");
}

TEST_F(DiagnosticsTest, InactiveDiags) {
  auto diag = diagEng.report(DiagID::unittest_placeholderremoval1, file);
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
  auto diagWarn = diagEng.report(DiagID::unittest_warntest, file);
  EXPECT_EQ(diagWarn.getSeverity(),DiagSeverity::Ignore) 
    << "Reported diagnostic wasn't a silenced diag";
  diagWarn.emit();
}

TEST_F(DiagnosticsTest, SilencedNotes) {
  // Set flag
  diagEng.setIgnoreNotes(true);
  // Test.
  auto diagNote = diagEng.report(DiagID::unittest_notetest, file);
  EXPECT_EQ(diagNote.getSeverity(), DiagSeverity::Ignore) 
    << "Reported diagnostic wasn't a silenced diag";
  diagNote.emit();
}

TEST_F(DiagnosticsTest, SilenceAllAfterFatal) {
  // Set flag
  diagEng.setIgnoreAllAfterFatal(true);
  // Test emission of an error
  diagEng.report(DiagID::unittest_errtest, file).emit();
  ASSERT_TRUE(diagEng.hadAnyError()) << "Error wasn't recorded?";
  
  // Report a fatal error
  diagEng.report(DiagID::unittest_fataltest, file).emit();
  EXPECT_TRUE(diagEng.hadFatalError());

  // And try to emit another error
  auto diagErrSilenced = diagEng.report(DiagID::unittest_errtest, file);
  EXPECT_EQ(diagErrSilenced.getSeverity(), DiagSeverity::Ignore)
    << "Diag was supposed to be silenced";
  diagErrSilenced.emit();
}

TEST_F(DiagnosticsTest, SilenceAll) {
  diagEng.setIgnoreAll(true);
  auto dg1 = diagEng.report(DiagID::unittest_errtest, file);
  auto dg2 = diagEng.report(DiagID::unittest_warntest, file);
  auto dg3 = diagEng.report(DiagID::unittest_fataltest, file);

  EXPECT_EQ(dg1.getSeverity(), DiagSeverity::Ignore) 
    << "Diag was supposed to be silenced.";
  EXPECT_EQ(dg2.getSeverity(), DiagSeverity::Ignore) 
    << "Diag was supposed to be silenced.";
  EXPECT_EQ(dg3.getSeverity(), DiagSeverity::Ignore) 
    << "Diag was supposed to be silenced.";
}

TEST_F(DiagnosticsTest, WarningsAreErrors) {
  diagEng.setWarningsAreErrors(true);
  diagEng.report(DiagID::unittest_warntest, file).emit();
  EXPECT_TRUE(diagEng.hadAnyError());
}

TEST_F(DiagnosticsTest, CopyingDiagKillsCopiedDiag) {
  // Test with copy constructor
  auto diagA = diagEng.report(DiagID::unittest_errtest, file);
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

TEST_F(PrettyDiagConsumerTest, PrintTest) {
  std::string theString = "LLVM is great!";
  FileID theFile = srcMgr.loadFromString(
    theString
  , "tmp");
  ASSERT_TRUE(theFile) << "String not correctly loaded in the SrcMgr!";
  // Test
  auto makeLoc = [theFile](std::size_t idx){return SourceLoc(theFile, idx);};
  // rangeA is "LLVM"
  SourceRange rangeA = SourceRange(makeLoc(0), makeLoc(3));
  // rangeB is "great!"
  SourceRange rangeB = SourceRange(makeLoc(8), makeLoc(13));
  // locPTE is past the end
  SourceLoc locPTE = makeLoc(14);
  // Diagnostic test at rangeA
  {
    diagEng.report(DiagID::unittest_notetest, rangeA);
    std::cout << "diag:" << ss.str() << "\n";
    EXPECT_EQ(ss.str(), 
      "<tmp>:1:1-4 - note - Test note\n"
      "    LLVM is great!\n"
      "    ^^^^\n"
    );
  
    ss.str("");
  }

  // Diagnostic test at rangeB
  {
    diagEng.report(DiagID::unittest_notetest, rangeB);
    std::cout << "diag:" << ss.str() << "\n";
    EXPECT_EQ(ss.str(), 
      "<tmp>:1:9-14 - note - Test note\n"
      "    LLVM is great!\n"
      "            ^^^^^^\n"
    );
  
    ss.str("");
  }

  // Diagnostic test at locPTE
  {
    diagEng.report(DiagID::unittest_notetest, locPTE);
    std::cout << "diag:" << ss.str() << "\n";
    EXPECT_EQ(ss.str(), 
      "<tmp>:1:15 - note - Test note\n"
      "    LLVM is great!\n"
      "                  ^\n"
    );
  
    ss.str("");
  }

  // Diagnostic test at locPTE + rangeA
  {
    diagEng.report(DiagID::unittest_notetest, locPTE)
           .setExtraRange(rangeA);
    std::cout << "diag:" << ss.str() << "\n";
    EXPECT_EQ(ss.str(), 
      "<tmp>:1:15 - note - Test note\n"
      "    LLVM is great!\n"
      "    ~~~~          ^\n"
    );
  
    ss.str("");
  }
}

TEST_F(PrettyDiagConsumerTest, UTF8PrintTest) {
  std::string theString = u8"This is a test: Γειά σου Κόσμε!";
  FileID theFile = srcMgr.loadFromString(
    theString
  , "tmp");
  ASSERT_TRUE(theFile) << "String not correctly loaded in the SrcMgr!";
  // Test
  auto makeLoc = [theFile](std::size_t idx){return SourceLoc(theFile, idx);};
  // Loc a is the space between the first unicode char
  SourceLoc locA(makeLoc(15));
  // rangeA is "Γειά σου Κόσμε" (14 chars, 26 bytes)
  // To calculate it, we substract the size of ε (2) = 24 
  SourceRange rangeA = SourceRange(makeLoc(16), makeLoc(40));
  // locB is the '!' after the unicode
  SourceLoc locB(makeLoc(42));

  std::stringstream compSS;

  // Diagnostic test at locA
  {
    diagEng.report(DiagID::unittest_notetest, locA);

    compSS << u8"<tmp>:1:16 - note - Test note\n"
           << u8"    This is a test: Γειά σου Κόσμε!\n";

    EXPECT_EQ(ss.str(), compSS.str());
  
    ss.str("");
    compSS.str("");
  }
    // Diagnostic test at rangeA
  {
    diagEng.report(DiagID::unittest_notetest, rangeA);

    compSS << u8"<tmp>:1:17-30 - note - Test note\n"
           << u8"    This is a test: Γειά σου Κόσμε!\n";

    EXPECT_EQ(ss.str(), compSS.str());
  
    ss.str("");
    compSS.str("");
  }

  // Diagnostic test at locB
  {
    diagEng.report(DiagID::unittest_notetest, locB);

    compSS << u8"<tmp>:1:31 - note - Test note\n"
           << u8"    This is a test: Γειά σου Κόσμε!\n";

    EXPECT_EQ(ss.str(), compSS.str());
  
    ss.str("");
    compSS.str("");
  }
}