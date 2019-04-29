//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BuiltinsTests.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
//  Unit tests for the getTypeOfBuiltin, Builtin Identifiers and 
//  BuiltinFuncDecl node.
//----------------------------------------------------------------------------//

#include "gtest/gtest.h"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/BuiltinsTypes.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/Types.hpp"
#include "Fox/Common/SourceManager.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include <iostream>

using namespace fox;

class BuiltinsTest : public ::testing::Test {
  public:
    BuiltinsTest() : diags(srcMgr, std::cout), ctxt(srcMgr, diags) {}

    SourceManager srcMgr;
    DiagnosticEngine diags;
    ASTContext ctxt;

};

TEST_F(BuiltinsTest, getTypeOfBuiltinTest) {
  Type printIntTy = getTypeOfBuiltin(ctxt, BuiltinID::printInt);
  Type printBoolTy = getTypeOfBuiltin(ctxt, BuiltinID::printBool);

  // Is 'toDebugString' fine for this?
  EXPECT_EQ(printIntTy->toDebugString(),  "(int) -> void");
  EXPECT_EQ(printBoolTy->toDebugString(), "(bool) -> void");
}

TEST_F(BuiltinsTest, builtinIdentifier) {
  EXPECT_EQ(ctxt.getIdentifier(BuiltinID::printBool).getStr(), 
            "printBool");
  EXPECT_EQ(ctxt.getIdentifier(BuiltinID::printBool),
            ctxt.getIdentifier(BuiltinID::printBool));

  EXPECT_EQ(ctxt.getIdentifier(BuiltinID::printInt).getStr(), 
            "printInt");
  EXPECT_EQ(ctxt.getIdentifier(BuiltinID::printInt),
            ctxt.getIdentifier(BuiltinID::printInt));
}

TEST_F(BuiltinsTest, builtinLookup) {
  auto printBool = ctxt.getIdentifier(BuiltinID::printBool);
  auto printInt = ctxt.getIdentifier(BuiltinID::printInt);
  {
    SmallVector<ValueDecl*, 4> results;
    ctxt.lookupBuiltin(printBool, results);
    ASSERT_EQ(results.size(), 1u) 
      << "Incorrect number of results for " << printBool.getStr();
    EXPECT_EQ(results.front(), BuiltinFuncDecl::get(ctxt, BuiltinID::printBool));
  }
  {
    SmallVector<ValueDecl*, 4> results;
    ctxt.lookupBuiltin(printInt, results);
    ASSERT_EQ(results.size(), 1u) 
      << "Incorrect number of results for " << printInt.getStr();
    EXPECT_EQ(results.front(), BuiltinFuncDecl::get(ctxt, BuiltinID::printInt));
  }
}
