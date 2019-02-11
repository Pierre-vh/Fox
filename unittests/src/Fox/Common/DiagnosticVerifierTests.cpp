//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : DiagnosticVerifierTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/Common/DiagnosticVerifier.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/SourceManager.hpp"
#include "Support/TestUtils.hpp"

using namespace fox;

namespace {
  class DVTest : public ::testing::Test {
    void error(const std::string str) {
      errStr = str;
      ok = false;
    }
    public:
      SourceManager srcMgr;
      DiagnosticEngine diags;
      DiagnosticVerifier dv;
      FileID file;
      bool ok = true;
      std::string errStr;
      DVTest(): diags(srcMgr), dv(DiagnosticVerifier(diags, srcMgr)) {}

      using ::testing::Test::SetUp;

      virtual void SetUp(const std::string& path, 
        std::unique_ptr<DiagnosticConsumer> consumer = nullptr) {
        if(consumer)
          diags.setConsumer(std::move(consumer));
        auto result = srcMgr.readFile(path);
        file = result.first;
        if (!file) {
          error("Couldn't load file \"" + path + "\"\n\tReason:" + 
            toString(result.second));
          return;
        }
        diags.enableVerifyMode(&dv);
        if (!dv.parseFile(file)) {
          error("The DiagnosticVerifier couldn't parse the file \"" + path + '"');
          return;
        }
      }      
  };

  class TestConsumer : public DiagnosticConsumer {
    string_view expected;
    public:
      TestConsumer(string_view expected) : expected(expected) {}
      
      std::size_t count = 0;
      bool ok = true;

      virtual void consume(SourceManager&, const Diagnostic& diag) override {
        ++count;
        ok &= (diag.getStr() != expected);
      }
  };
}

TEST_F(DVTest, Parsing) {
  using DV = DiagSeverity;

  SetUp(test::getPath("diagnosticsverifier/parse.txt"));
  ASSERT_TRUE(ok) << errStr;
  auto& diags = dv.getExpectedDiags();
  ASSERT_EQ(diags.size(), 4u) << "Incorrect number of verify instrs found";
 
  bool foundNote = false;
  bool foundWarn = false;
  bool foundError = false;
  bool foundFatal = false;
  for (auto& diag : diags) {
    EXPECT_EQ(diag.file, file) << "File mismatch";
    if (diag.severity == DV::Note) {
      EXPECT_EQ(diag.line, 1u) << "Line mismatch";
      EXPECT_EQ(diag.diagStr, "Note ipsum dolor sit") << "DiagStr mismatch";
      foundNote = true;
    }
    else if (diag.severity == DV::Warning) {
      EXPECT_EQ(diag.line, 2u) << "Line mismatch";
      EXPECT_EQ(diag.diagStr, "Lorem Warn dolor sit") << "DiagStr mismatch";
      foundWarn = true;
    }
    else if (diag.severity == DV::Error) {
      EXPECT_EQ(diag.line, 3u) << "Line mismatch";
      EXPECT_EQ(diag.diagStr, "Lorem ipsum Errror sit") << "DiagStr mismatch";
      foundError = true;
    }
    else if (diag.severity == DV::Fatal) {
      EXPECT_EQ(diag.line, 4u) << "Line mismatch";
      EXPECT_EQ(diag.diagStr, "Lorem ipsum dolor Fatal") << "DiagStr mismatch";
      foundFatal = true;
    } 
    else {
      FAIL() << "Unknown/Unhandled severity:" << diag.severity;
    }
  }
  
  EXPECT_TRUE(foundNote) << "Note verify instr not parsed";
  EXPECT_TRUE(foundWarn) << "Warning verify instr not parsed";
  EXPECT_TRUE(foundError) << "Error verify instr not parsed";
  EXPECT_TRUE(foundFatal) << "Fatal verify instr not parsed";
}

TEST_F(DVTest, Trim) {
  SetUp(test::getPath("diagnosticsverifier/trim.txt"));
  ASSERT_TRUE(ok) << errStr;
  auto& diags = dv.getExpectedDiags();
  for (auto& diag : diags) {
    EXPECT_EQ(diag.diagStr, "Lorem ipsum dolor sit amet") 
      << "Incorrect/Untrimmed diag string";
  }
}

TEST_F(DVTest, Offset) {
  SetUp(test::getPath("diagnosticsverifier/offset.txt"));
  ASSERT_TRUE(ok) << errStr;
  auto& diags = dv.getExpectedDiags();
  ASSERT_EQ(diags.size(), 19u) << "Incorrect number of verify instrs found";

  for (auto& diag : diags) {
    // Every diag's line should be line 10.
    ASSERT_EQ(diag.line, 10u);
  }
}

TEST_F(DVTest, BadOffset) {
  {
    auto newConsumer = std::make_unique<TestConsumer>("foobar");
    SetUp(test::getPath("diagnosticsverifier/badoffset.txt"), 
      std::move(newConsumer));
  }
  auto diagConsumer = static_cast<TestConsumer*>(diags.getConsumer());
  ASSERT_FALSE(ok);
  auto& diags = dv.getExpectedDiags();
  ASSERT_EQ(diags.size(), 0u) << "Incorrect number of verify instrs found";
  ASSERT_EQ(diagConsumer->count, 2u) << "Incorrect number of diags emitted";
  ASSERT_TRUE(diagConsumer->ok) << "Unknown diags emitted";
}
