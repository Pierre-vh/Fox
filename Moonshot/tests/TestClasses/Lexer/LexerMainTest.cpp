////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : LexerMainTest.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "LexerMainTest.h"

using namespace Moonshot;
using namespace Moonshot::TestUtilities;

LexerMainTest::~LexerMainTest()
{
}

bool LexerMainTest::runTest(Context & context)
{
	auto correct_test_str = readFileToString(context,"tests\\lexer\\lexer_correct.fox");
	auto bad_test_str = readFileToString(context, "tests\\lexer\\lexer_bad.fox");
	// Run correct tests 
	Lexer lex_corr(context);
	lex_corr.lexStr(correct_test_str);
	if (!context.isSafe_strict()) // Should be 0 errors
		return false;
	// Run "bad" tests
	Lexer lex_bad(context);
	lex_bad.lexStr(bad_test_str); 
	if (context.isSafe_strict()) // Should have errors
		return false;
	// All went well, reset context state and return.
	context.resetState();
	return true;
}

std::string LexerMainTest::getTestName() const
{
	return "Lexer main test";
}

